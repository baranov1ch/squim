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

#ifndef SQUIM_APP_IMAGE_OPTIMIZER_CLIENT_H_
#define SQUIM_APP_IMAGE_OPTIMIZER_CLIENT_H_

#include <memory>

#include "grpc++/grpc++.h"
#include "proto/image_optimizer.grpc.pb.h"
#include "squim/base/make_noncopyable.h"

namespace io {
class Reader;
class Writer;
}

class RequestBuilder;

class ImageOptimizerClient {
  MAKE_NONCOPYABLE(ImageOptimizerClient);

 public:
  ImageOptimizerClient(std::shared_ptr<grpc::Channel> channel);

  bool OptimizeImage(RequestBuilder* request_builder,
                     io::Reader* image_reader,
                     size_t chunk_size,
                     io::Writer* webp_writer,
                     squim::ImageResponsePart_Stats* stats);

 private:
  std::unique_ptr<squim::ImageOptimizer::Stub> stub_;
};

#endif  // SQUIM_APP_IMAGE_OPTIMIZER_CLIENT_H_
