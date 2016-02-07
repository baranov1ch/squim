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

#include "squim/app/optimizers/squim_webp.h"

#include "squim/base/logging.h"

image::WebPEncoder::Compression ToWebPCompression(
    squim::ImageRequestPart::WebPCompressionType type) {
  switch (type) {
    case squim::ImageRequestPart::AUTO:
      return image::WebPEncoder::Compression::kLossy;
    case squim::ImageRequestPart::LOSSY:
      return image::WebPEncoder::Compression::kLossy;
    case squim::ImageRequestPart::LOSSLESS:
      return image::WebPEncoder::Compression::kLossless;
    case squim::ImageRequestPart::MIXED:
      return image::WebPEncoder::Compression::kMixed;
    default:
      NOTREACHED();
      return image::WebPEncoder::Compression::kLossy;
  }
}

SquimWebP::SquimWebP(const squim::ImageRequestPart_Meta& request)
    : request_(request) {}

bool SquimWebP::ShouldWaitForMetadata() {
  // TODO: mess with metadata.
  return false;
}

void SquimWebP::AdjustWebPEncoderParams(image::WebPEncoder::Params* params) {
  // TODO: set progress callback to handle timeout.
  if (request_.has_webp_params()) {
    const auto& webp_params = request_.webp_params();
    if (webp_params.quality() > 0)
      params->quality = webp_params.quality();

    if (webp_params.method() > 0)
      params->method = webp_params.method();
    params->write_stats = webp_params.record_stats();
    if (webp_params.compression_type() != squim::ImageRequestPart::AUTO) {
      params->compression = ToWebPCompression(webp_params.compression_type());
    }
  }
}
