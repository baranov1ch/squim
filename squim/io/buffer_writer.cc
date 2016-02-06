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

#include "squim/io/buffer_writer.h"

#include <cstring>

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"

namespace io {

BufferWriter::WritableChunk::WritableChunk(size_t size)
    : RawChunk(std::unique_ptr<uint8_t[]>(new uint8_t[size]), size) {}

BufferWriter::WritableChunk::~WritableChunk() {}

void BufferWriter::WritableChunk::Write(uint8_t* in_data, size_t size) {
  std::memcpy(data() + offset_, in_data, size);
  offset_ += size;
}

void BufferWriter::WritableChunk::UnwriteN(size_t size) {
  offset_ -= size;
}

BufferWriter::BufferWriter(size_t chunk_size)
    : chunk_size_(chunk_size), current_chunk_(chunks_.end()) {}

BufferWriter::~BufferWriter() {}

IoResult BufferWriter::Write(Chunk* chunk) {
  return Write(chunk->data(), chunk->size());
}

IoResult BufferWriter::Write(uint8_t* data, size_t size) {
  if (current_chunk_ == chunks_.end()) {
    chunks_.push_back(base::make_unique<WritableChunk>(chunk_size_));
    current_chunk_ = chunks_.begin();
  }

  size_t nwrite = 0;
  while (nwrite < size) {
    if ((*current_chunk_)->available() == 0) {
      if (std::next(current_chunk_) == chunks_.end())
        chunks_.push_back(base::make_unique<WritableChunk>(chunk_size_));
      current_chunk_++;
    }
    auto& chunk = *current_chunk_;

    auto effective_size = std::min(size - nwrite, chunk->available());
    chunk->Write(data + nwrite, effective_size);
    nwrite += effective_size;
  }

  total_size_ += size;
  return IoResult::Write(size);
}

size_t BufferWriter::UnwriteN(size_t n) {
  size_t nunwritten = 0;
  while (nunwritten < n) {
    auto& chunk = *current_chunk_;
    auto eff_unwrite_size = std::min(n - nunwritten, chunk->offset());
    chunk->UnwriteN(eff_unwrite_size);
    nunwritten += eff_unwrite_size;
    if (chunk->offset() == 0) {
      if (current_chunk_ == chunks_.begin())
        break;

      current_chunk_--;
    }
  }

  total_size_ -= nunwritten;
  return nunwritten;
}

ChunkList BufferWriter::ReleaseChunks() {
  ChunkList ret;
  auto start = chunks_.begin();
  auto it = chunks_.begin();
  for (it = start; it != chunks_.end(); ++it) {
    auto& chunk = *it;
    auto offset = chunk->offset();
    if (offset == 0)
      break;

    if (chunk->available() == 0) {
      ret.push_back(std::move(chunk));
    } else {
      ret.push_back(Chunk::Wrap(std::move(chunk), 0, offset));
    }
  }
  current_chunk_ = chunks_.erase(start, it);
  total_size_ = 0;
  return ret;
}

}  // namespace io
