#include "io/chunk.h"

namespace io {

Chunk::Chunk(const uint8_t* data, uint64_t size)
    : data_(const_cast<uint8_t*>(data)), size_(size) {}

StringChunk::StringChunk(std::string data)
    : Chunk(reinterpret_cast<const uint8_t*>(data.data()), data.size()),
      holder_(std::move(data)) {}

StringChunk::~StringChunk() {}

RawChunk::RawChunk(std::unique_ptr<uint8_t[]> data, uint64_t size)
    : Chunk(data.get(), size), data_(std::move(data)) {}

RawChunk::~RawChunk() {}

}  // namespace io
