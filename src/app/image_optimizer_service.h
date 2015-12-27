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

#ifndef APP_IMAGE_OPTIMIZER_SERVICE_H_
#define APP_IMAGE_OPTIMIZER_SERVICE_H_

#include "grpc++/grpc++.h"
#include "proto/image_optimizer.grpc.pb.h"
#include "proto/image_optimizer.pb.h"

class ImageOptimizerService final : public squim::ImageOptimizer::Service {
  grpc::Status OptimizeImage(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<squim::ImageResponsePart,
                               squim::ImageRequestPart>* stream) override;
};

#endif  // APP_IMAGE_OPTIMIZER_SERVICE_H_
