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

#include "squim/image/codecs/webp/multiframe_webp_encoder.h"

#include "squim/base/logging.h"
#include "squim/image/codecs/webp/webp_util.h"
#include "squim/image/image_frame.h"
#include "squim/image/image_info.h"
#include "squim/image/pixel.h"
#include "squim/io/writer.h"

namespace image {

MultiframeWebPEncoder::MultiframeWebPEncoder(WebPEncoder::Params* params,
                                             io::VectorWriter* output)
    : params_(params), output_(output) {
  memset(&webp_image_, 0, sizeof(WebPPicture));
  memset(&webp_frame_, 0, sizeof(WebPPicture));
}

MultiframeWebPEncoder::~MultiframeWebPEncoder() {
  WebPFrameCacheDelete(webp_frame_cache_);
  WebPMuxDelete(webp_mux_);
  WebPPictureFree(&webp_image_);
}

void MultiframeWebPEncoder::SetImageInfo(const ImageInfo* image_info) {
  image_info_ = image_info;
}

void MultiframeWebPEncoder::SetMetadata(const ImageMetadata* metadata) {
  metadata_ = metadata;
}

Result MultiframeWebPEncoder::EncodeFrame(ImageFrame* frame) {
  if (!webp_mux_) {
    auto result = InitMuxer();
    if (!result.ok())
      return result;
  }

  auto result =
      EncoderParamsToWebPConfig(*params_, &webp_config_, frame->quality());
  if (!result.ok())
    return result;

  if (!WebPPictureView(&webp_image_, frame->x_offset(), frame->y_offset(),
                       frame->width(), frame->height(), &webp_frame_))
    return Result::Error(Result::Code::kEncodeError,
                         WebPError("WebPPictureView: ", &webp_image_));

  auto* where = webp_frame_.argb;
  Bitmap bitmap(frame);
  switch (frame->color_scheme()) {
    case ColorScheme::kGrayScale:
      for (uint32_t y = 0; y < frame->height(); ++y) {
        for (uint32_t x = 0; x < frame->width(); ++x) {
          auto px = bitmap.GetPixel<GrayScalePixel>(x, y);
          *where++ = PackAsARGB(px.g(), px.g(), px.g(), 0xFF);
        }
      }
      break;
    case ColorScheme::kGrayScaleAlpha:
      for (uint32_t y = 0; y < frame->height(); ++y) {
        for (uint32_t x = 0; x < frame->width(); ++x) {
          auto px = bitmap.GetPixel<GrayScaleAlphaPixel>(x, y);
          *where++ = PackAsARGB(px.g(), px.g(), px.g(), px.a());
        }
      }
      break;
    case ColorScheme::kRGB:
      for (uint32_t y = 0; y < frame->height(); ++y) {
        for (uint32_t x = 0; x < frame->width(); ++x) {
          auto px = bitmap.GetPixel<RGBPixel>(x, y);
          *where++ = PackAsARGB(px.r(), px.g(), px.b(), 0xFF);
        }
      }
      break;
    case ColorScheme::kRGBA:
      for (uint32_t y = 0; y < frame->height(); ++y) {
        for (uint32_t x = 0; x < frame->width(); ++x) {
          auto px = bitmap.GetPixel<RGBAPixel>(x, y);
          *where++ = PackAsARGB(px.r(), px.g(), px.b(), px.a());
        }
      }
      break;
    default:
      return Result::Error(Result::Code::kEncodeError,
                           "WebP encode: unsupported color scheme");
  }

  // We need to pass image to add frame.
  WebPFrameRect frame_rect = {
      static_cast<int>(frame->x_offset()), static_cast<int>(frame->y_offset()),
      static_cast<int>(frame->width()), static_cast<int>(frame->height())};
  auto disposal_method = DisposalMethodToWebPDisposal(frame->disposal_method());

  if (!WebPFrameCacheAddFrame(webp_frame_cache_, &webp_config_, &frame_rect,
                              disposal_method, frame->duration(),
                              &webp_image_)) {
    return Result::Error(Result::Code::kEncodeError,
                         WebPError("WebPFrameCacheAddFrame: ", &webp_image_));
  }

  if (WebPFrameCacheFlush(webp_frame_cache_, false /*verbose*/, webp_mux_) !=
      WEBP_MUX_OK)
    return Result::Error(Result::Code::kEncodeError,
                         "WebPFrameCacheFlush error");

  next_frame_idx_++;

  return Result::Ok();
}

Result MultiframeWebPEncoder::FinishEncoding() {
  if (WebPFrameCacheFlushAll(webp_frame_cache_, false /*verbose*/, webp_mux_) !=
      WEBP_MUX_OK)
    return Result::Error(Result::Code::kEncodeError,
                         "WebPFrameCacheFlushAll error");

  if (next_frame_idx_ > 1) {
    RGBAPixel bg(const_cast<uint8_t*>(image_info_->bg_color->data()));
    // This was an animated image.
    WebPMuxAnimParams anim = {PackAsARGB(bg.r(), bg.g(), bg.b(), bg.a()),
                              static_cast<int>(image_info_->loop_count)};
    if (WebPMuxSetAnimationParams(webp_mux_, &anim) != WEBP_MUX_OK)
      return Result::Error(Result::Code::kEncodeError,
                           "WebPMuxSetAnimationParams error");
  }

  if (params_->should_write_metadata() && !metadata_->Empty()) {
    SetMetadataIfNeeded(params_->write_iccp, "ICCP", ImageMetadata::Type::kICC);
    SetMetadataIfNeeded(params_->write_iccp, "EXIF",
                        ImageMetadata::Type::kEXIF);
    SetMetadataIfNeeded(params_->write_iccp, "XMP ", ImageMetadata::Type::kXMP);
  }

  WebPData webp_data = {NULL, 0};
  if (WebPMuxAssemble(webp_mux_, &webp_data) != WEBP_MUX_OK)
    return Result::Error(Result::Code::kEncodeError,
                         WebPError("WebPMuxAssemble: ", &webp_image_));

  auto chunk = io::Chunk::Copy(webp_data.bytes, webp_data.size);
  io::ChunkList chunks;
  chunks.push_back(std::move(chunk));
  auto write_result = output_->WriteV(std::move(chunks));

  WebPDataClear(&webp_data);

  return Result::FromIoResult(write_result, false);
}

void MultiframeWebPEncoder::GetStats(ImageOptimizationStats* stats) {
  if (!stats_)
    return;
}

Result MultiframeWebPEncoder::InitMuxer() {
  CHECK(image_info_);

  webp_mux_ = WebPMuxNew();
  if (!webp_mux_)
    return Result::Error(Result::Code::kEncodeError, "WebP mux create error");

  webp_image_.width = image_info_->width;
  webp_image_.height = image_info_->height;
  webp_image_.use_argb = true;

  if (!WebPPictureAlloc(&webp_image_))
    return Result::Error(Result::Code::kEncodeError,
                         WebPError("WebPPictureAlloc: ", &webp_image_));

  WebPUtilClearPic(&webp_image_, nullptr);

  webp_image_.progress_hook = WebPProgressHook;
  webp_image_.user_data = params_;

  // Key frame parameters: do not insert unnecessary key frames.
  static const size_t kMax = ~0;
  static const size_t kMin = kMax - 1;
  webp_frame_cache_ = WebPFrameCacheNew(
      image_info_->width, image_info_->height, kMin, kMax,
      params_->compression == WebPEncoder::Compression::kMixed);

  return Result::Ok();
}

void MultiframeWebPEncoder::SetMetadataIfNeeded(bool needed,
                                                const char fourcc[4],
                                                ImageMetadata::Type type) {
  if (needed && metadata_->IsCompleted(type)) {
    auto chunk = io::Chunk::Merge(metadata_->Get(type));
    WebPData meta{chunk->data(), chunk->size()};
    WebPMuxSetChunk(webp_mux_, fourcc, &meta, 1);
  }
}

}  // namespace image
