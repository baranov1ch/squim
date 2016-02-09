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

#include "squim/app/image_optimizer_service.h"

#include "squim/app/optimization.h"
#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/base/strings/string_util.h"
#include "squim/image/optimization/image_optimizer.h"
#include "squim/io/buf_reader.h"
#include "squim/io/buf_writer.h"
#include "squim/io/buffered_source.h"
#include "squim/io/chunk.h"
#include "squim/io/writer.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using squim::ImageRequestPart;
using squim::ImageResponsePart;

namespace {

class ChunkBuffer : public io::VectorWriter {
 public:
  ChunkBuffer(size_t chunk_size) {
    auto receiver = base::make_unique<Receiver>(this);
    writer_ = base::make_unique<io::BufWriter>(chunk_size, std::move(receiver));
  }

  io::IoResult WriteV(io::ChunkList chunks) override {
    auto nwrite = 0;
    for (auto& chunk : chunks) {
      auto result = writer_->Write(chunk.get());
      DCHECK(result.ok());
      DCHECK_EQ(chunk->size(), result.n());
      nwrite += result.n();
    }
    return io::IoResult::Write(nwrite);
  }

  void Flush() {
    auto result = writer_->Flush();
    DCHECK(!result.pending());
  }

  bool Empty() const { return buf_chain_.empty(); }

  io::ChunkPtr PopChunk() {
    DCHECK(!Empty());
    auto chunk = std::move(buf_chain_.front());
    buf_chain_.pop_front();
    return chunk;
  }

 private:
  class Receiver : public io::Writer {
   public:
    Receiver(ChunkBuffer* chunk_buffer) : chunk_buffer_(chunk_buffer) {}

    io::IoResult Write(io::Chunk* chunk) override {
      chunk_buffer_->AddChunk(chunk->Clone());
      return io::IoResult::Write(chunk->size());
    }

   private:
    ChunkBuffer* chunk_buffer_;
  };

  void AddChunk(io::ChunkPtr chunk) { buf_chain_.push_back(std::move(chunk)); }

  io::ChunkList buf_chain_;
  std::unique_ptr<io::BufWriter> writer_;
};

class SyncRequestHandler {
 public:
  SyncRequestHandler(
      Optimization* optimization,
      ServerReaderWriter<ImageResponsePart, ImageRequestPart>* stream)
      : optimization_(optimization), stream_(stream) {}

  Status Handle() {
    while (true) {
      auto request_part = base::make_unique<ImageRequestPart>();
      if (!stream_->Read(request_part.get()))
        break;

      if (!optimizer_) {
        if (!request_part->has_meta()) {
          stream_->Write(CreateError(ImageResponsePart::CONTRACT_ERROR));
          return Status::OK;
        }

        if (!ProcessHeader(*request_part)) {
          stream_->Write(CreateError(ImageResponsePart::REJECTED));
          return Status::OK;
        }
      } else {
        if (!request_part->has_image_data()) {
          stream_->Write(CreateError(ImageResponsePart::CONTRACT_ERROR));
          return Status::OK;
        }

        auto result = ProcessData(std::move(request_part));
        if (result.error()) {
          stream_->Write(CreateError(ImageResponsePart::ENCODE_ERROR));
          return Status::OK;
        }

        if (result.finished())
          break;

        DrainOutput();
      }
    }

    auto result = optimizer_->Process();
    DCHECK(result.finished());
    output_->Flush();
    DrainOutput();

    ImageResponsePart trailer;
    const auto& optimization_stats = optimizer_->stats();
    auto* stats = trailer.mutable_stats();
    stats->set_psnr(optimization_stats.psnr);
    stats->set_coded_size(optimization_stats.coded_size);
    // TODO: send stats.
    stream_->Write(trailer);

    return Status::OK;
  }

 private:
  void DrainOutput() {
    while (!output_->Empty()) {
      if (!response_started_) {
        response_started_ = true;
        ImageResponsePart response;
        auto* meta = response.mutable_meta();
        meta->set_code(ImageResponsePart::OK);
        stream_->Write(response);
      }
      auto chunk = output_->PopChunk();
      ImageResponsePart response;
      auto* image_data = response.mutable_image_data();
      image_data->set_bytes(
          base::StringFromBytes(chunk->data(), chunk->size()).as_string());
      stream_->Write(response);
    }
  }

  // TODO: more error description.
  bool ProcessHeader(const ImageRequestPart& header) {
    const auto& meta = header.meta();
    if (meta.target_type() != squim::WEBP)
      return false;

    auto strategy = optimization_->CreateOptimizationStrategy(meta);
    auto src = io::BufReader::CreateEmpty();
    input_ = src.get();
    auto dst = base::make_unique<ChunkBuffer>(4096);
    output_ = dst.get();
    optimizer_.reset(new image::ImageOptimizer(
        image::ImageOptimizer::DefaultImageTypeSelector, std::move(strategy),
        std::move(src), std::move(dst)));
    return true;
  }

  image::Result ProcessData(std::unique_ptr<ImageRequestPart> data) {
    DCHECK(optimizer_);
    DCHECK(input_);
    std::string copy(data->image_data().bytes());
    input_->source()->AddChunk(io::Chunk::FromString(std::move(copy)));
    return optimizer_->Process();
  }

  ImageResponsePart CreateError(ImageResponsePart::Result result) {
    ImageResponsePart error;
    auto* meta = error.mutable_meta();
    meta->set_code(result);
    return error;
  }

  Optimization* optimization_;
  ServerReaderWriter<ImageResponsePart, ImageRequestPart>* stream_;
  std::unique_ptr<image::ImageOptimizer> optimizer_;
  io::BufReader* input_ = nullptr;
  ChunkBuffer* output_ = nullptr;
  bool response_started_ = false;
};

}  // namespace

ImageOptimizerService::ImageOptimizerService(
    std::unique_ptr<Optimization> optimization)
    : optimization_(std::move(optimization)) {}

Status ImageOptimizerService::OptimizeImage(
    ServerContext* context,
    ServerReaderWriter<ImageResponsePart, ImageRequestPart>* stream) {
  return SyncRequestHandler(optimization_.get(), stream).Handle();
}
