/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "squim/image/codecs/webp/simple_webp_encoder.h"

#include <cstring>

#include <chrono>
#include <functional>

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/codecs/webp/webp_util.h"
#include "squim/image/image_frame.h"
#include "squim/image/image_info.h"
#include "squim/image/image_metadata.h"
#include "squim/image/image_optimization_stats.h"
#include "squim/image/pixel.h"
#include "squim/io/buf_reader.h"
#include "squim/io/buf_writer.h"
#include "squim/io/buffered_source.h"
#include "squim/io/writer.h"
#include "squim/ioutil/chunk_writer.h"

namespace image {

namespace {

const size_t kTagSize = 4;
const size_t kChunkHeaderSize = 8;

class LEWriter {
 public:
  LEWriter(io::Writer* underlying) : underlying_(underlying) {}

  io::IoResult Write(io::Chunk* chunk) { return underlying_->Write(chunk); }

  template <size_t N>
  io::IoResult WriteBytes(const char(&bytes)[N]) {
    return WriteBytes(bytes, N);
  }

  template <size_t N>
  io::IoResult WriteBytes(const uint8_t(&bytes)[N]) {
    return WriteBytes(bytes, N);
  }

  io::IoResult WriteBytes(const char* bytes, size_t len) {
    return WriteBytes(reinterpret_cast<const uint8_t*>(bytes), len);
  }

  io::IoResult WriteBytes(const uint8_t* bytes, size_t len) {
    io::Chunk chunk(const_cast<uint8_t*>(bytes), len);
    return Write(&chunk);
  }

  io::IoResult WriteLE(uint32_t val, size_t num) {
    uint8_t buf[4];
    for (size_t i = 0; i < num; ++i) {
      buf[i] = static_cast<uint8_t>(val & 0xFF);
      val >>= 8;
    }
    return WriteBytes(buf, num);
  }

  io::IoResult WriteLE32(uint32_t val) { return WriteLE(val, 4); }

  io::IoResult WriteLE24(uint32_t val) { return WriteLE(val, 3); }

 private:
  io::Writer* underlying_;
};

io::ChunkList CreateMetadataPayload(const char* fourcc, io::ChunkPtr data) {
  io::ChunkList ret;
  ioutil::ChunkListWriter underlying(&ret);
  LEWriter le_writer(&underlying);
  auto result = le_writer.WriteBytes(fourcc, 4);
  DCHECK_EQ(kTagSize, result.n());
  bool need_padding = data->size() & 1;
  le_writer.WriteLE32(data->size());
  ret.push_back(std::move(data));
  if (need_padding) {
    static const uint8_t kZero[1] = {0};
    ret.push_back(io::Chunk::View(const_cast<uint8_t*>(kZero), 1));
  }
  return ret;
}

io::ChunkPtr PrepareMetdataChunk(bool needed,
                                 const ImageMetadata* metadata,
                                 ImageMetadata::Type type,
                                 uint32_t flag,
                                 uint32_t* out_flags,
                                 size_t* size) {
  if (!needed || !metadata->IsCompleted(type))
    return io::ChunkPtr();

  auto chunk = io::Chunk::Merge(metadata->Get(type));
  *size += chunk->size();
  *out_flags |= flag;
  return chunk;
}

class defer {
 public:
  defer(std::function<void()> fn) : fn_(fn) {}
  ~defer() { fn_(); }

 private:
  std::function<void()> fn_;
};

}  // namespace

SimpleWebPEncoder::SimpleWebPEncoder(WebPEncoder::Params* params,
                                     io::VectorWriter* output)
    : params_(params), output_(output) {
  memset(&picture_, 0, sizeof(WebPPicture));
}

SimpleWebPEncoder::~SimpleWebPEncoder() {
  if (owns_data_)
    WebPPictureFree(&picture_);
}

void SimpleWebPEncoder::SetImageInfo(const ImageInfo* image_info) {
  image_info_ = image_info;
}

void SimpleWebPEncoder::SetMetadata(const ImageMetadata* metadata) {
  metadata_ = metadata;
}

Result SimpleWebPEncoder::EncodeFrame(ImageFrame* frame) {
  auto start = std::chrono::high_resolution_clock::now();
  defer d([start]() {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                        std::chrono::high_resolution_clock::now() - start)
                        .count();
    VLOG(1) << "Encoding duration: " << duration << " us";
  });
  auto conf_result =
      EncoderParamsToWebPConfig(*params_, &webp_config_, frame->quality());
  if (!conf_result.ok())
    return conf_result;

  std::unique_ptr<ImageFrame> transformed_frame;
  if (frame->is_grayscale()) {
    // TODO: transform to RGB(A).
    transformed_frame.reset(new ImageFrame);
    ConvertGrayToRGB(frame, transformed_frame.get());
    frame = transformed_frame.get();
  }

  if (!frame->is_yuv() && !frame->is_rgb())
    return Result::Error(Result::Code::kEncodeError,
                         "Invalid color scheme for webp encoding");

  picture_.width = frame->width();
  picture_.height = frame->height();

  bool result = false;
  switch (frame->color_scheme()) {
    case ColorScheme::kRGB:
      result =
          WebPPictureImportRGB(&picture_, frame->GetData(0), frame->stride());
      owns_data_ = true;
      break;
    case ColorScheme::kRGBA:
      result =
          WebPPictureImportRGBA(&picture_, frame->GetData(0), frame->stride());
      owns_data_ = true;
      break;
    case ColorScheme::kYUV:
    case ColorScheme::kYUVA:
      result = WebPPictureFromYUVAFrame(frame, &picture_);
      break;
    default:
      NOTREACHED();
  }

  if (!result)
    return Result::Error(Result::Code::kEncodeError,
                         WebPError("WebP picture import error: ", &picture_));

  picture_.writer = ChunkWriter;
  picture_.custom_ptr = this;
  picture_.progress_hook = WebPProgressHook;
  picture_.user_data = params_;

  if (params_->write_stats) {
    stats_ = base::make_unique<WebPAuxStats>();
    picture_.stats = stats_.get();
  }

  // Now take picture and WebP encode it.
  result = WebPEncode(&webp_config_, &picture_);

  if (!result)
    return Result::Error(Result::Code::kEncodeError,
                         WebPError("WebP encode error: ", &picture_));

  return Result::Ok();
}

Result SimpleWebPEncoder::FinishEncoding() {
  if (chunks_.empty())
    return Result::Ok();

  if (!params_->should_write_metadata() || metadata_->Empty()) {
    auto io_result = output_->WriteV(std::move(chunks_));
    chunks_ = io::ChunkList();
    return Result::FromIoResult(io_result, false);
  }

  const uint32_t kAlphaFlag = 0x10;
  const uint32_t kEXIFFlag = 0x08;
  const uint32_t kICCPFlag = 0x20;
  const uint32_t kXMPFlag = 0x04;
  const size_t kVP8XChunkSize = 18;
  const size_t kRiffHeaderSize = 12;
  const char kVP8XHeader[] = "VP8X\x0a\x00\x00\x00";

  size_t metadata_total_size = 0;
  uint32_t flags = 0;

  auto iccp = PrepareMetdataChunk(params_->write_iccp, metadata_,
                                  ImageMetadata::Type::kICC, kICCPFlag, &flags,
                                  &metadata_total_size);
  auto exif = PrepareMetdataChunk(params_->write_exif, metadata_,
                                  ImageMetadata::Type::kEXIF, kEXIFFlag, &flags,
                                  &metadata_total_size);
  auto xmp = PrepareMetdataChunk(params_->write_xmp, metadata_,
                                 ImageMetadata::Type::kXMP, kXMPFlag, &flags,
                                 &metadata_total_size);

  auto source = base::make_unique<io::BufferedSource>(std::move(chunks_));
  auto reader = base::make_unique<io::BufReader>(std::move(source));
  chunks_ = io::ChunkList();

  auto webp_size = reader->source()->size();
  auto result = reader->SkipN(kRiffHeaderSize);
  DCHECK(!result.pending());
  uint8_t vp8_tag[kTagSize];
  result = reader->PeekNInto(vp8_tag);
  DCHECK(!result.pending());
  bool has_vp8x = std::memcmp(vp8_tag, "VP8X", kTagSize) == 0;
  auto riff_size = webp_size - kChunkHeaderSize + metadata_total_size;
  if (!has_vp8x)
    riff_size += kVP8XChunkSize;

  io::BufWriter buf_writer(kRiffHeaderSize + kVP8XChunkSize + 1,
                           std::unique_ptr<io::Writer>());
  LEWriter writer(&buf_writer);
  writer.WriteBytes("RIFF", kTagSize);
  writer.WriteLE32(riff_size);
  writer.WriteBytes("WEBP", kTagSize);
  if (has_vp8x) {
    uint8_t* vp8x;
    result = reader->ReadN(&vp8x, kVP8XChunkSize);
    DCHECK(!result.pending());
    DCHECK_EQ(kVP8XChunkSize, result.n());
    vp8x[kChunkHeaderSize] |= static_cast<uint8_t>(flags & 0xFF);
    writer.WriteBytes(vp8x, kVP8XChunkSize);
  } else {
    bool is_lossless = std::memcmp(vp8_tag, "VP8L", kTagSize) == 0;
    if (is_lossless) {
      // Presence of alpha is stored in the 29th bit of VP8L data.
      // Thus we read chunk header + 32 bits == 8 + 4 bytes.
      uint8_t vp8l[kChunkHeaderSize + 4];
      result = reader->PeekNInto(vp8l);
      DCHECK(!result.pending());
      if (vp8l[kChunkHeaderSize + 3] & (1 << 5))
        flags |= kAlphaFlag;
    }
    writer.WriteBytes(kVP8XHeader);
    writer.WriteLE32(flags);
    writer.WriteLE24(picture_.width - 1);
    writer.WriteLE24(picture_.height - 1);
  }

  io::ChunkList final_webp;
  final_webp.push_back(buf_writer.ReleaseBuffer());
  if (iccp) {
    auto iccp_data = CreateMetadataPayload("ICCP", std::move(iccp));
    final_webp.splice(final_webp.end(), iccp_data);
  }
  auto webp_body = reader->source()->ReleaseRest();
  final_webp.splice(final_webp.end(), webp_body);
  if (exif) {
    auto exif_data = CreateMetadataPayload("EXIF", std::move(exif));
    final_webp.splice(final_webp.end(), exif_data);
  }
  if (xmp) {
    auto xmp_data = CreateMetadataPayload("XMP ", std::move(xmp));
    final_webp.splice(final_webp.end(), xmp_data);
  }

  auto io_result = output_->WriteV(std::move(final_webp));
  return Result::FromIoResult(io_result, false);
}

void SimpleWebPEncoder::GetStats(ImageOptimizationStats* stats) {
  if (!stats_)
    return;

  stats->psnr = stats_->PSNR[3];
  stats->coded_size = stats_->coded_size;
}

// static
int SimpleWebPEncoder::ChunkWriter(const uint8_t* data,
                                   size_t data_size,
                                   const WebPPicture* const picture) {
  auto* encoder = static_cast<SimpleWebPEncoder*>(picture->custom_ptr);
  auto chunk = io::Chunk::Copy(data, data_size);
  encoder->chunks_.push_back(std::move(chunk));
  return 1;
}

}  // namespace image
