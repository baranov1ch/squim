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

#ifndef IO_BUFFER_WRITER_H_
#define IO_BUFFER_WRITER_H_

#include <cstdint>
#include <memory>

#include "io/chunk.h"
#include "io/io_result.h"

namespace io {

class BufferWriter {
 public:
  BufferWriter(size_t chunk_size);
  ~BufferWriter();

  IoResult Write(Chunk* chunk);
  IoResult Write(uint8_t* data, size_t size);
  size_t UnwriteN(size_t n);

  ChunkList ReleaseChunks();

  size_t total_size() const { return total_size_; }

 private:
  class WritableChunk : public RawChunk {
   public:
    WritableChunk(size_t size);
    ~WritableChunk();

    void Write(uint8_t* in_data, size_t size);
    void UnwriteN(size_t size);

    size_t available() const { return size() - offset(); }
    size_t offset() const { return offset_; }

   private:
    size_t offset_ = 0;
  };

  size_t chunk_size_;
  size_t total_size_ = 0;
  using WritableChunkPtr = std::unique_ptr<WritableChunk>;
  using WritableChunkList = std::list<WritableChunkPtr>;
  WritableChunkList chunks_;
  WritableChunkList::iterator current_chunk_;
};

}  // namespace io

#endif  // IO_BUFFER_WRITER_H_
