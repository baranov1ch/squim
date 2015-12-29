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

#include "app/image_optimizer_client.h"

#include <thread>
#include <algorithm>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "proto/image_optimizer.pb.h"

using squim::ImageOptimizer;
using squim::ImageRequestPart;
using squim::ImageResponsePart;

ImageOptimizerClient::ImageOptimizerClient(
    std::shared_ptr<grpc::Channel> channel)
    : stub_(squim::ImageOptimizer::NewStub(channel)) {}

bool ImageOptimizerClient::OptimizeImage(const std::vector<uint8_t>& image_data,
                                         size_t chunk_size,
                                         std::vector<uint8_t>* webp_data) {
  grpc::ClientContext context;

  std::shared_ptr<grpc::ClientReaderWriter<ImageRequestPart, ImageResponsePart>>
      stream(stub_->OptimizeImage(&context));

  std::thread writer([stream, &image_data, chunk_size]() {
    ImageRequestPart header;
    header.set_type(ImageRequestPart::META);
    auto* meta = header.mutable_meta();
    meta->set_target_type(squim::WEBP);
    auto* webp_params = meta->mutable_webp_params();
    webp_params->set_quality(40.0);
    webp_params->set_strength(4);
    webp_params->set_compression_type(ImageRequestPart::LOSSY);
    stream->Write(header);

    size_t offset = 0;
    size_t rest = image_data.size();
    while (rest > 0) {
      auto effective_len = std::min(rest, chunk_size);
      ImageRequestPart data;
      data.set_type(ImageRequestPart::IMAGE_DATA);
      data.set_data(base::StringFromBytes(&image_data[offset], effective_len)
                        .as_string());
      rest -= effective_len;
      offset += effective_len;
      stream->Write(data);
    }
  });

  bool result = true;
  webp_data->clear();
  ImageResponsePart response_part;
  while (stream->Read(&response_part)) {
    if (response_part.type() == ImageResponsePart::META) {
      if (response_part.meta().code() != ImageResponsePart::OK) {
        result = false;
        LOG(ERROR) << "ImageOptimizer optimization failed";
        break;
      }
    } else if (response_part.type() == ImageResponsePart::IMAGE_DATA) {
      const uint8_t* data =
          reinterpret_cast<const uint8_t*>(response_part.data().data());
      webp_data->insert(webp_data->end(), data,
                        data + response_part.data().size());
    } else {
      // TODO: handle stats.
    }
  }
  writer.join();
  auto status = stream->Finish();
  if (!status.ok()) {
    LOG(ERROR) << "ImageOptimizer rpc failed.";
    return false;
  }

  return result;
}
