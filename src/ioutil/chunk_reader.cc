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

#include "ioutil/chunk_reader.h"

#include <algorithm>
#include <cstring>

#include "base/logging.h"
#include "io/chunk.h"

namespace ioutil {

ChunkReader::ChunkReader(const io::Chunk* source) : source_(source) {
  DCHECK(source_);
}

io::IoResult ChunkReader::Read(io::Chunk* chunk) {
  if (offset_ >= source_->size())
    return io::IoResult::Eof();

  auto eff_len = std::min(chunk->size(), source_->size() - offset_);
  std::memcpy(chunk->data(), source_->data() + offset_, eff_len);
  offset_ += eff_len;
  return io::IoResult::Read(eff_len);
}

ChunkListReader::ChunkListReader(const io::ChunkList* source)
    : source_(source) {
  current_chunk_ = source_->begin();
}

io::IoResult ChunkListReader::Read(io::Chunk* chunk) {
  if (current_chunk_ == source_->end())
    return io::IoResult::Eof();

  size_t nread = 0;
  size_t offset = 0;
  while (offset < chunk->size() && current_chunk_ != source_->end()) {
    auto& chunk_to_read = *current_chunk_;
    auto eff_len = std::min(chunk->size() - offset,
                            chunk_to_read->size() - offset_in_chunk_);
    std::memcpy(chunk->data() + offset,
                chunk_to_read->data() + offset_in_chunk_, eff_len);
    offset += eff_len;
    offset_in_chunk_ += eff_len;
    nread += eff_len;

    if (offset_in_chunk_ == chunk_to_read->size()) {
      // Move to the next chunk in the |source_|.
      offset_in_chunk_ = 0;
      current_chunk_++;
    }
  }
  return io::IoResult::Read(nread);
}

}  // namespace ioutil
