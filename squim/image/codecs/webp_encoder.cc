/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#include "squim/image/codecs/webp_encoder.h"

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/codecs/webp/multiframe_webp_encoder.h"
#include "squim/image/codecs/webp/simple_webp_encoder.h"
#include "squim/io/writer.h"

namespace image {

// static
WebPEncoder::Params WebPEncoder::Params::Default() {
  return Params();
}

WebPEncoder::WebPEncoder(Params params, std::unique_ptr<io::VectorWriter> dst)
    : params_(params), dst_(std::move(dst)) {}

WebPEncoder::~WebPEncoder() {}

Result WebPEncoder::Initialize(const ImageInfo* image_info) {
  image_info_ = image_info;
  if (impl_)
    impl_->SetImageInfo(image_info_);

  return Result::Ok();
}

Result WebPEncoder::EncodeFrame(ImageFrame* frame, bool last_frame) {
  if (!error_.ok())
    return error_;

  if (!impl_) {
    if (last_frame && frame) {
      impl_ = base::make_unique<SimpleWebPEncoder>(&params_, dst_.get());
    } else {
      impl_ = base::make_unique<MultiframeWebPEncoder>(&params_, dst_.get());
    }
  }

  impl_->SetImageInfo(image_info_);
  impl_->SetMetadata(metadata_);

  if (frame) {
    auto result = impl_->EncodeFrame(frame);
    if (result.error())
      error_ = result;

    return result;
  }

  return Result::Ok();
}

void WebPEncoder::SetMetadata(const ImageMetadata* metadata) {
  metadata_ = metadata;
  if (impl_)
    impl_->SetMetadata(metadata_);
}

Result WebPEncoder::FinishWrite(ImageOptimizationStats* stats) {
  auto result = impl_->FinishEncoding();
  if (result.error()) {
    error_ = result;
    return result;
  }

  impl_->GetStats(stats);
  return result;
}

}  // namespace image
