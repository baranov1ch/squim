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

#include "squim/app/request_builder.h"

RequestBuilder::RequestBuilder() {
  auto* webp_params = request_.mutable_meta()->mutable_webp_params();
  webp_params->set_quality(50.0);
}

RequestBuilder& RequestBuilder::SetQuality(double quality) {
  request_.mutable_meta()->mutable_webp_params()->set_quality(quality);
  return *this;
}

RequestBuilder& RequestBuilder::SetWebPMethod(int method) {
  request_.mutable_meta()->mutable_webp_params()->set_method(method);
  return *this;
}

RequestBuilder& RequestBuilder::SetWebPCompression(
    squim::ImageRequestPart::WebPCompressionType type) {
  request_.mutable_meta()->mutable_webp_params()->set_compression_type(type);
  return *this;
}

RequestBuilder& RequestBuilder::SetRecordStats(bool record_stats) {
  request_.mutable_meta()->mutable_webp_params()->set_record_stats(
      record_stats);
  return *this;
}

squim::ImageRequestPart RequestBuilder::Build() {
  request_.mutable_meta()->set_target_type(squim::WEBP);
  return request_;
}
