#ifndef IO_IO_BUFFERED_SOURCE_H_
#define IO_IO_BUFFERED_SOURCE_H_

#include <list>
#include <memory>

namespace io {

class Chunk;

class BufferedSource {
 public:
  BufferedSource();
  ~BufferedSource();

  // Checks if there are some data.
  bool HasSome() const;

  // Checks if EOF was received and all data has been read.
  bool EofReached() const;

  // Checks if there are |n| bytes immediately available.
  bool HasN(uint64_t n);

  // Returns largest available continuous chunk. You MUST check
  // that HasSome returned |true| before calling this.
  void ReadSome(uint8_t** out, uint64_t* length);

  // Returns continuous data of the size |n|. Merges chunks if necessary.
  // You MUST check that HasN(n) returned |true| before calling this.
  void ReadN(uint8_t** out, uint64_t n);

  void AddChunk(std::unique_ptr<Chunk> chunk);

  // Tries to remove data from the front of the data. Does not reallocate,
  // removes chunks only if they fully fit |n|.
  void RemoveAtMostNBytes(uint64_t n);

 private:
  using ChunkList = std::list<std::unique_ptr<Chunk>>;

  ChunkList::iterator current_chunk_;
  ChunkList chunks_;
  bool eof_received_ = false;
  uint64_t total_offset_ = 0u;
  uint64_t offset_in_chunk_ = 0u;
};

}  // namespace io

#endif  // IO_IO_BUFFERED_SOURCE_H_
