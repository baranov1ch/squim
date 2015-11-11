#ifndef IO_CHUNK_H_
#define IO_CHUNK_H_

#include <memory>
#include <string>

namespace io {

// Abstract chunk of data.
class Chunk {
 public:
  Chunk(const uint8_t* data, uint64_t size);
  virtual ~Chunk() {}

  const uint8_t* data() const { return data_; }
  const char* char_data() const { return reinterpret_cast<const char*>(data_); }
  uint8_t* data() { return data_; }
  uint64_t size() const { return size_; }

 private:
  uint8_t* data_;
  uint64_t size_;
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
  explicit RawChunk(std::unique_ptr<uint8_t[]> data, uint64_t size);
  ~RawChunk() override;

 private:
  std::unique_ptr<uint8_t[]> data_;
};

}  // namespace io

#endif  // IO_CHUNK_H_
