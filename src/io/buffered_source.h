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

#ifndef IO_BUFFERED_SOURCE_H_
#define IO_BUFFERED_SOURCE_H_

#include <list>
#include <memory>

#include "io/chunk.h"

namespace io {

class BufferedSource {
 public:
  BufferedSource();
  ~BufferedSource();

  // Checks if there are some data.
  bool HaveSome() const;

  // Checks if EOF was received and all data has been read.
  bool EofReached() const;

  // Checks if there are |n| bytes immediately available.
  bool HaveN(size_t n);

  // Returns largest available continuous chunk. You MUST check
  // that HasSome returned |true| before calling this.
  // Returns number of bytes read.
  size_t ReadSome(uint8_t** out);

  // Returns continuous chunk not more than |desired| size. You MUST check
  // that HasSome returned |true| before calling this.
  // Returns number of bytes read.
  size_t ReadAtMostN(uint8_t** out, size_t desired);

  // Returns continuous data of the size |n|. Merges chunks if necessary.
  // You MUST check that HasN(n) returned |true| before calling this.
  // Returns number of bytes read (just in case to be consistent).
  size_t ReadN(uint8_t** out, size_t n);

  void AddChunk(ChunkPtr chunk);

  // Tries to remove already consumed data from the front of the data. Does
  // not reallocate, removes chunks only if they fully fit |n|.
  // Returns number of bytes freed.
  size_t FreeAtMostNBytes(size_t n);

  // Frees all chunks up to active. Returns number of bytes freed.
  size_t FreeAsMuchAsPossible();

  // Unreads |n| bytes from source, or less if buffer is shorter. Returns the
  // number of bytes unread.
  size_t UnreadN(size_t n);

  // Closes the source. No data will be accepted after this call.
  void SendEof();

  size_t offset() const { return total_offset_; }
  size_t size() const { return total_size_; }

 private:
  ChunkList chunks_;
  ChunkList::iterator current_chunk_;
  bool eof_received_ = false;
  size_t total_offset_ = 0u;
  size_t offset_in_chunk_ = 0u;
  size_t total_size_ = 0u;
};

}  // namespace io

#endif  // IO_BUFFERED_SOURCE_H_
