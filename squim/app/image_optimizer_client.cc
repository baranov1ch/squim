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

#include "squim/app/image_optimizer_client.h"

#include <thread>
#include <algorithm>

#include "proto/image_optimizer.pb.h"
#include "squim/app/request_builder.h"
#include "squim/base/logging.h"
#include "squim/base/optional.h"
#include "squim/base/strings/string_util.h"
#include "squim/io/chunk.h"
#include "squim/io/reader.h"
#include "squim/io/writer.h"
#include "squim/ioutil/read_util.h"

using squim::ImageOptimizer;
using squim::ImageRequestPart;
using squim::ImageResponsePart;
using squim::ImageResponsePart_Stats;

typedef std::shared_ptr<
    grpc::ClientReaderWriter<ImageRequestPart, ImageResponsePart>>
    ImageStreamPtr;

namespace {
class GRPCStreamWriter : public io::Writer {
 public:
  GRPCStreamWriter(ImageStreamPtr stream) : stream_(stream) {}

  io::IoResult Write(io::Chunk* chunk) {
    auto* data = body_.mutable_image_data();
    data->set_bytes(chunk->data(), chunk->size());
    stream_->Write(body_);
    return io::IoResult::Write(chunk->size());
  }

 private:
  ImageRequestPart body_;
  ImageStreamPtr stream_;
};

class GRPCStreamReader : public io::Reader {
 public:
  GRPCStreamReader(ImageStreamPtr stream, ImageResponsePart* response_part)
      : stream_(stream), response_part_(response_part) {}

  io::IoResult Read(io::Chunk* chunk) {
    if (!offset_) {
      while (true) {
        if (!stream_->Read(response_part_))
          return io::IoResult::Eof();

        if (response_part_->has_stats()) {
          stats_ = response_part_->stats();
          continue;
        }

        if (!response_part_->has_image_data())
          return io::IoResult::Error("Unexpected gRPC message");

        break;
      }

      offset_ = 0;
    }

    const auto& bytes = response_part_->image_data().bytes();
    auto effective_len = std::min(bytes.size(), chunk->size());
    std::memcpy(chunk->data(), bytes.data() + *offset_, effective_len);
    *offset_ += effective_len;

    if (*offset_ == bytes.size())
      offset_.reset();

    return io::IoResult::Read(effective_len);
  }

  const ImageResponsePart_Stats& stats() const { return stats_; }

 private:
  ImageStreamPtr stream_;
  ImageResponsePart* response_part_;
  base::optional<size_t> offset_;
  ImageResponsePart_Stats stats_;
};
}

ImageOptimizerClient::ImageOptimizerClient(
    std::shared_ptr<grpc::Channel> channel)
    : stub_(squim::ImageOptimizer::NewStub(channel)) {}

bool ImageOptimizerClient::OptimizeImage(RequestBuilder* request_builder,
                                         io::Reader* image_reader,
                                         size_t chunk_size,
                                         io::Writer* webp_writer,
                                         ImageResponsePart_Stats* stats) {
  grpc::ClientContext context;
  ImageStreamPtr stream(stub_->OptimizeImage(&context));
  auto header = request_builder->Build();

  std::thread writer([header, stream, &image_reader, chunk_size]() {
    stream->Write(header);

    GRPCStreamWriter writer(stream);
    auto result = ioutil::Copy(&writer, image_reader, chunk_size);
    if (!result.ok())
      LOG(ERROR) << "Image read/send error: " << result.message();
  });

  ImageResponsePart response_part;
  stream->Read(&response_part);
  if (!response_part.has_meta()) {
    LOG(ERROR) << "Unexpected gRPC message: No status part";
    return false;
  }

  if (response_part.meta().code() != ImageResponsePart::OK) {
    LOG(ERROR) << "Optimization failed: " << response_part.meta().message();
    return false;
  }

  GRPCStreamReader reader(stream, &response_part);
  auto result = ioutil::Copy(webp_writer, &reader);

  if (stats)
    *stats = reader.stats();

  writer.join();
  auto status = stream->Finish();
  if (!status.ok()) {
    LOG(ERROR) << "Final RPC failed, but optimization succeeded";
    return false;
  }

  return result.ok();
}
