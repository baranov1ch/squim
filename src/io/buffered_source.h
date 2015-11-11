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
  bool HaveSome() const;

  // Checks if EOF was received and all data has been read.
  bool EofReached() const;

  // Checks if there are |n| bytes immediately available.
  bool HaveN(uint64_t n);

  // Returns largest available continuous chunk. You MUST check
  // that HasSome returned |true| before calling this.
  // Returns number of bytes read.
  uint64_t ReadSome(uint8_t** out);

  // Returns continuous chunk not more than |desired| size. You MUST check
  // that HasSome returned |true| before calling this.
  // Returns number of bytes read.
  uint64_t ReadAtMostN(uint8_t** out, uint64_t desired);

  // Returns continuous data of the size |n|. Merges chunks if necessary.
  // You MUST check that HasN(n) returned |true| before calling this.
  // Returns number of bytes read (just in case to be consistent).
  uint64_t ReadN(uint8_t** out, uint64_t n);

  void AddChunk(std::unique_ptr<Chunk> chunk);

  // Tries to remove already consumed data from the front of the data. Does
  // not reallocate, removes chunks only if they fully fit |n|.
  // Returns number of bytes freed.
  uint64_t FreeAtMostNBytes(uint64_t n);

  // Frees all chunks up to active. Returns number of bytes freed.
  uint64_t FreeAsMuchAsPossible();

  // Unreads |n| bytes from source, or less if buffer is shorter. Returns the
  // number of bytes unread.
  uint64_t UnreadN(uint64_t n);

  // Closes the source. No data will be accepted after this call.
  void SendEof();

  uint64_t offset() const { return total_offset_; }
  uint64_t size() const { return total_size_; }

 private:
  using ChunkList = std::list<std::unique_ptr<Chunk>>;

  ChunkList chunks_;
  ChunkList::iterator current_chunk_;
  bool eof_received_ = false;
  uint64_t total_offset_ = 0u;
  uint64_t offset_in_chunk_ = 0u;
  uint64_t total_size_ = 0u;
};

}  // namespace io

#endif  // IO_IO_BUFFERED_SOURCE_H_
