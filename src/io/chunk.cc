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

#include "io/chunk.h"

#include <cstring>

#include "base/memory/make_unique.h"
#include "base/strings/string_util.h"

namespace io {

ChunkPtr Chunk::FromString(std::string data) {
  return base::make_unique<StringChunk>(std::move(data));
}

ChunkPtr Chunk::Copy(const uint8_t* data, size_t size) {
  if (size == 0)
    return base::make_unique<Chunk>(nullptr, 0);

  std::unique_ptr<uint8_t[]> owned_data(new uint8_t[size]);
  std::memcpy(owned_data.get(), data, size);
  return Own(std::move(owned_data), size);
}

ChunkPtr Chunk::View(uint8_t* data, size_t size) {
  return base::make_unique<Chunk>(data, size);
}

ChunkPtr Chunk::Own(std::unique_ptr<uint8_t[]> data, size_t size) {
  return base::make_unique<RawChunk>(std::move(data), size);
}

ChunkPtr Chunk::New(size_t size) {
  if (size == 0)
    return base::make_unique<Chunk>(nullptr, 0);

  std::unique_ptr<uint8_t[]> owned_data(new uint8_t[size]);
  return Own(std::move(owned_data), size);
}

ChunkPtr Chunk::Wrap(ChunkPtr to_wrap, size_t start, size_t size) {
  return base::make_unique<WrappingChunk>(std::move(to_wrap), start, size);
}

ChunkPtr Chunk::Merge(const ChunkList& chunks) {
  size_t total_size = 0;
  for (const auto& chunk : chunks)
    total_size += chunk->size();

  auto result = Chunk::New(total_size);
  size_t offset = 0;
  for (const auto& chunk : chunks) {
    std::memcpy(result->data() + offset, chunk->data(), chunk->size());
    offset += chunk->size();
  }
  return std::move(result);
}

Chunk::Chunk(const uint8_t* data, size_t size)
    : data_(const_cast<uint8_t*>(data)), size_(size) {}

base::StringPiece Chunk::ToString() const {
  return base::StringFromBytes(data_, size_);
}

ChunkPtr Chunk::Clone() {
  return Chunk::Copy(data_, size_);
}

ChunkPtr Chunk::Slice(size_t start, size_t size) {
  return Chunk::View(data_ + start, size);
}

StringChunk::StringChunk(std::string data)
    : Chunk(reinterpret_cast<const uint8_t*>(data.data()), data.size()),
      holder_(std::move(data)) {}

StringChunk::~StringChunk() {}

RawChunk::RawChunk(std::unique_ptr<uint8_t[]> data, size_t size)
    : Chunk(data.get(), size), data_(std::move(data)) {}

RawChunk::~RawChunk() {}

WrappingChunk::WrappingChunk(ChunkPtr wrapped, size_t start, size_t size)
    : Chunk(wrapped->data() + start, size), wrapped_(std::move(wrapped)) {}

WrappingChunk::~WrappingChunk() {}

}  // namespace io
