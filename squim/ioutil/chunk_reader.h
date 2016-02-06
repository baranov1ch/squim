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

#ifndef SQUIM_IOUTIL_CHUNK_READER_H_
#define SQUIM_IOUTIL_CHUNK_READER_H_

#include "squim/io/chunk.h"
#include "squim/io/reader.h"

namespace ioutil {

class ChunkReader : public io::Reader {
 public:
  explicit ChunkReader(const io::Chunk* source);

  io::IoResult Read(io::Chunk* chunk) override;

 private:
  const io::Chunk* source_;
  size_t offset_ = 0;
};

class ChunkListReader : public io::Reader {
 public:
  explicit ChunkListReader(const io::ChunkList* source);

  io::IoResult Read(io::Chunk* chunk) override;

 private:
  const io::ChunkList* source_;
  size_t offset_in_chunk_ = 0;
  io::ChunkList::const_iterator current_chunk_;
};

}  // namespace ioutil

#endif  // SQUIM_IOUTIL_CHUNK_READER_H_
