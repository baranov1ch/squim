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

#include "app/optimization.h"
#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "base/strings/string_util.h"
#include "image/optimization/image_optimizer.h"
#include "io/buf_reader.h"
#include "io/buffered_source.h"
#include "io/chunk.h"
#include "io/writer.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using squim::ImageRequestPart;
using squim::ImageResponsePart;

namespace {

class ProtoChunk : public io::Chunk {
 public:
  ProtoChunk(std::unique_ptr<ImageRequestPart> chunk)
      : Chunk(base::BytesFromConstChar(chunk->data().data()),
              chunk->data().size()) {
    DCHECK_EQ(ImageRequestPart::IMAGE_DATA, chunk->type());
  }

 private:
  std::unique_ptr<ImageRequestPart> chunk_;
};

class BufWriter : public io::Writer {
 public:
  BufWriter(size_t buf_size, std::unique_ptr<io::Writer> underlying)
      : buffer_(io::Chunk::New(buf_size)), underlying_(std::move(underlying)) {}

  io::IoResult Write(io::Chunk* chunk) override {
    if (flushing_)
      return io::IoResult::Pending();

    size_t nwrite = 0;
    for (;;) {
      auto effective_len = std::min(available(), chunk->size());
      auto to_copy = chunk->Slice(nwrite, effective_len);

      std::memcpy(buffer_->data() + offset_, to_copy->data(), effective_len);
      offset_ += effective_len;
      nwrite += effective_len;

      if (available() == 0) {
        auto result = Flush();
        if (result.error())
          return result;

        if (result.pending())
          return io::IoResult::Write(nwrite);

        DCHECK_EQ(0, start_);
        DCHECK_EQ(0, offset_);
      }

      if (nwrite == chunk->size())
        return io::IoResult::Write(nwrite);
    }

    NOTREACHED();
    return io::IoResult::Error();
  }

  io::IoResult Flush() {
    flushing_ = true;

    size_t nwrite = 0;
    for (;;) {
      auto to_write = buffer_->Slice(start_, offset_ - start_);
      auto result = underlying_->Write(to_write.get());
      if (result.pending()) {
        return result;
      }

      if (result.error()) {
        flushing_ = false;
        return result;
      }

      start_ += result.n();
      nwrite += result.n();
      if (start_ == offset_) {
        // Success.
        start_ = 0;
        offset_ = 0;
        flushing_ = false;
        return io::IoResult::Write(nwrite);
      }
    }

    NOTREACHED();
    return io::IoResult::Error();
  }

  size_t available() const { return buffer_->size() - offset_; }
  size_t buffered() const { return offset_; }

 private:
  io::ChunkPtr buffer_;
  size_t start_ = 0;
  size_t offset_ = 0;
  bool flushing_ = false;
  std::unique_ptr<io::Writer> underlying_;
};

class ChunkBuffer : public io::VectorWriter {
 public:
  ChunkBuffer(size_t chunk_size) {
    auto receiver = base::make_unique<Receiver>(this);
    writer_ = base::make_unique<BufWriter>(chunk_size, std::move(receiver));
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

  size_t chunk_size_;
  io::ChunkList buf_chain_;
  std::unique_ptr<BufWriter> writer_;
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
        if (request_part->type() != ImageRequestPart::META) {
          stream_->Write(CreateError(ImageResponsePart::CONTRACT_ERROR));
          return Status::OK;
        }

        if (!ProcessHeader(*request_part)) {
          stream_->Write(CreateError(ImageResponsePart::REJECTED));
          return Status::OK;
        }
      } else {
        if (request_part->type() != ImageRequestPart::IMAGE_DATA) {
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

      auto result = optimizer_->Process();
      DCHECK(result.finished());
      output_->Flush();
      DrainOutput();

      ImageResponsePart stats;
      stream_->Write(stats);
    }

    return Status::OK;
  }

 private:
  void DrainOutput() {
    while (!output_->Empty()) {
      if (!response_started_) {
        response_started_ = true;
        ImageResponsePart header;
        header.set_type(ImageResponsePart::META);
        auto* meta = header.mutable_meta();
        meta->set_code(ImageResponsePart::OK);
        stream_->Write(header);
      }
      auto chunk = output_->PopChunk();
      ImageResponsePart data;
      data.set_type(ImageResponsePart::IMAGE_DATA);
      data.set_data(
          base::StringFromBytes(chunk->data(), chunk->size()).as_string());
      stream_->Write(data);
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
    input_->source()->AddChunk(base::make_unique<ProtoChunk>(std::move(data)));
    return optimizer_->Process();
  }

  ImageResponsePart CreateError(ImageResponsePart::Result result) {
    ImageResponsePart error;
    error.set_type(ImageResponsePart::META);
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
