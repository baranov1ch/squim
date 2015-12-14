#include "io/chunk.h"

#include <cstring>

#include "base/memory/make_unique.h"
#include "base/strings/string_util.h"

namespace io {

Chunk::Chunk(const uint8_t* data, size_t size)
    : data_(const_cast<uint8_t*>(data)), size_(size) {}

base::StringPiece Chunk::ToString() const {
  return base::StringFromBytes(data_, size_);
}

ChunkPtr Chunk::FromString(std::string data) {
  return base::make_unique<StringChunk>(std::move(data));
}

ChunkPtr Chunk::Copy(const uint8_t* data, size_t size) {
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

StringChunk::StringChunk(std::string data)
    : Chunk(reinterpret_cast<const uint8_t*>(data.data()), data.size()),
      holder_(std::move(data)) {}

StringChunk::~StringChunk() {}

RawChunk::RawChunk(std::unique_ptr<uint8_t[]> data, size_t size)
    : Chunk(data.get(), size), data_(std::move(data)) {}

RawChunk::~RawChunk() {}

}  // namespace io
