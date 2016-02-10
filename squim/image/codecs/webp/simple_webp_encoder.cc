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

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/codecs/webp/webp_util.h"
#include "squim/image/image_frame.h"
#include "squim/image/image_info.h"
#include "squim/image/image_optimization_stats.h"
#include "squim/image/pixel.h"
#include "squim/io/writer.h"

namespace image {

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
  if (!chunks_.empty()) {
    auto write_result = output_->WriteV(std::move(chunks_));
    chunks_ = io::ChunkList();
    return Result::FromIoResult(write_result, false);
  }

  return Result::Ok();
}

Result SimpleWebPEncoder::FinishEncoding() {
  return Result::Ok();
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
