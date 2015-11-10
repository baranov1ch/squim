#ifndef IO_CHUNK_H_
#define IO_CHUNK_H_

#include <string>

namespace io {

// Abstract chunk of data.
class Chunk {
 public:
  Chunk(const uint8_t* data, uint64_t size);
  virtual ~Chunk() {}

  const uint8_t* data() const;
  uint8_t* data();
  uint64_t size() const;

 private:
  uint64_t size_;
  uint8_t* data_;
};

class StringChunk : public Chunk {
 public:
  explicit StringChunk(std::string data);
  ~StringChunk() override;

 private:
  std::string holder_;
};

}  // namespace io

#endif  // IO_CHUNK_H_
