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

#ifndef SQUIM_APP_REQUEST_BUILDER_H_
#define SQUIM_APP_REQUEST_BUILDER_H_

#include "proto/image_optimizer.pb.h"

class RequestBuilder {
 public:
  RequestBuilder();

  RequestBuilder& SetQuality(double quality);
  RequestBuilder& SetWebPMethod(int method);
  RequestBuilder& SetWebPCompression(
      squim::ImageRequestPart::WebPCompressionType type);
  RequestBuilder& SetRecordStats(bool record_stats);

  squim::ImageRequestPart Build();

 private:
  squim::ImageRequestPart request_;
};

#endif  // SQUIM_APP_REQUEST_BUILDER_H_
