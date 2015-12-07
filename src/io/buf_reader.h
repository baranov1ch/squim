#ifndef IO_IO_BUF_READER_H_
#define IO_IO_BUF_READER_H_

#include <cstdint>
#include <memory>

#include "io/io_result.h"

namespace io {

class BufferedSource;

// Reads data from the underlying |source_|. Tries not to do any copies.
class BufReader {
 public:
  BufReader(std::unique_ptr<BufferedSource> source);
  ~BufReader();

  // Sets |out| to the largest continuous piece of data available in |source_|.
  // Advances the offset for returned IoResult::n() bytes.
  // Do not copies any data, just returns a pointer.
  IoResult ReadSome(uint8_t** out);

  // Sets |out| to continuous piece of data of the size |n| or less.
  // Advances the offset for returned IoResult::n() bytes.
  // Do not copies any data, just returns a pointer.
  IoResult ReadAtMostN(uint8_t** out, size_t n);

  // Sets |out| to continuous piece of data of the size |n|.
  // Advances the offset for returned IoResult::n() bytes.
  // NOTE: If |source_| does not have enough continuous data, some of the chunks
  // will be merged, so data copies may occur during this call.
  // Returns error if |source_| ended (got EOF) earlier.
  IoResult ReadN(uint8_t** out, size_t n);

  // Copies into |out| piece of data of the size |n|.
  // Advances the offset for returned IoResult::n() bytes (which is |n|).
  // Returns error if |source_| ended (got EOF) earlier.
  IoResult ReadNInto(uint8_t* out, size_t n);

  template <size_t N>
  IoResult ReadNInto(uint8_t(&out)[N]) {
    return ReadNInto(out, N);
  }

  // Copies into |out| piece of data of the size |n|.
  // Does not advance the offset.
  IoResult PeekNInto(uint8_t* out, size_t n);

  template <size_t N>
  IoResult PeekNInto(uint8_t(&out)[N]) {
    return PeekNInto(out, N);
  }

  // Unreads n bytes from the buffer. Returns the number of bytes unread.
  size_t UnreadN(size_t n);

  BufferedSource* source() { return source_.get(); }
  size_t offset() const;
  bool HaveSome() const;

 private:
  std::unique_ptr<BufferedSource> source_;
};

}  // namespace io

#endif  // IO_IO_BUF_READER_H_
