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

#ifndef SQUIM_IO_CHUNK_H_
#define SQUIM_IO_CHUNK_H_

#include <list>
#include <memory>
#include <string>

#include "squim/base/strings/string_piece.h"

namespace io {

class Chunk;
using ChunkPtr = std::unique_ptr<Chunk>;
using ChunkList = std::list<ChunkPtr>;

// Abstract chunk of data.
class Chunk {
 public:
  Chunk(const uint8_t* data, size_t size);
  virtual ~Chunk() {}

  const uint8_t* data() const { return data_; }
  uint8_t* data() { return data_; }
  size_t size() const { return size_; }

  base::StringPiece ToString() const;

  ChunkPtr Clone();
  ChunkPtr Slice(size_t start, size_t size);

  static ChunkPtr FromString(std::string data);
  static ChunkPtr Copy(const uint8_t* data, size_t size);
  static ChunkPtr View(uint8_t* data, size_t size);
  static ChunkPtr Own(std::unique_ptr<uint8_t[]> data, size_t size);
  static ChunkPtr New(size_t size);
  static ChunkPtr Wrap(ChunkPtr to_wrap, size_t start, size_t size);
  static ChunkPtr Merge(const ChunkList& chunks);

 private:
  uint8_t* data_;
  size_t size_;
};

class StringChunk : public Chunk {
 public:
  explicit StringChunk(std::string data);
  ~StringChunk() override;

 private:
  std::string holder_;
};

class RawChunk : public Chunk {
 public:
  RawChunk(std::unique_ptr<uint8_t[]> data, size_t size);
  ~RawChunk() override;

 private:
  std::unique_ptr<uint8_t[]> data_;
};

class WrappingChunk : public Chunk {
 public:
  WrappingChunk(ChunkPtr wrapped, size_t start, size_t size);
  ~WrappingChunk() override;

 private:
  ChunkPtr wrapped_;
};

}  // namespace io

#endif  // SQUIM_IO_CHUNK_H_
