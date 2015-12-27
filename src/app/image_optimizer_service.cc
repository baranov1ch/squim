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

#include "app/image_optimizer_service.h"

#include "image/optimization/convert_to_webp_strategy.h"
#include "image/optimization/default_codec_factory.h"
#include "image/optimization/image_optimizer.h"
#include "image/optimization/strategy_builder.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using squim::ImageRequestPart;
using squim::ImageResponsePart;

Status ImageOptimizerService::OptimizeImage(
    ServerContext* context,
    ServerReaderWriter<ImageResponsePart, ImageRequestPart>* stream) {
  image::StrategyBuilder builder;
  auto strategy = builder.SetBaseStrategy<image::ConvertToWebPStrategy>(
                             image::DefaultCodecFactory::Builder)
                      .Build();
  strategy->ShouldEvenBother();
  ImageRequestPart request_part;
  while (stream->Read(&request_part)) {
    ImageResponsePart response_part;
    response_part.set_message("Hello, " + request_part.name());
    stream->Write(response_part);
  }
  return Status::OK;
}
