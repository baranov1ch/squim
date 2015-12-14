#ifndef IO_CHUNK_H_
#define IO_CHUNK_H_

#include <list>
#include <memory>
#include <string>

#include "base/strings/string_piece.h"

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

  static ChunkPtr FromString(std::string data);
  static ChunkPtr Copy(const uint8_t* data, size_t size);
  static ChunkPtr View(uint8_t* data, size_t size);
  static ChunkPtr Own(std::unique_ptr<uint8_t[]> data, size_t size);

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

}  // namespace io

#endif  // IO_CHUNK_H_
