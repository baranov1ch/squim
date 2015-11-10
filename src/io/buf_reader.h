#ifndef IO_IO_BUF_READER_H_
#define IO_IO_BUF_READER_H_

#include <memory>
#include <stdint.h>

#include "io/io_result.h"

namespace io {

class BufferedSource;

// Reads data from the underlying |source_|. Tries not to do any copies.
class BufReader {
 public:
  BufReader(std::unique_ptr<BufferedSource> source);

  // Sets |out| to the largest continuous piece of data available in |source_|.
  // Advances the offset for returned IoResult::nread() bytes.
  // Do not copies any data, just returns a pointer.
  IoResult ReadSome(uint8_t** out);

  // Sets |out| to continuous piece of data of the size |n| or less.
  // Advances the offset for returned IoResult::nread() bytes.
  // Do not copies any data, just returns a pointer.
  IoResult ReadAtMostN(uint8_t** out, uint64_t n);

  // Sets |out| to continuous piece of data of the size |n|.
  // Advances the offset for returned IoResult::nread() bytes.
  // NOTE: If |source_| does not have enough continuous data, some of the chunks
  // will be merged, so data copies may occur during this call.
  // Returns error if |source_| ended (got EOF) earlier.
  IoResult ReadAtLeastN(uint8_t** out, uint64_t n);

  // Copies into |out| piece of data of the size |n|.
  // Advances the offset for returned IoResult::nread() bytes (which is |n|).
  // Returns error if |source_| ended (got EOF) earlier.
  IoResult ReadAtLeastNInto(uint8_t* out, uint64_t n);

  // Copies into |out| piece of data of the size |n|.
  // Do not advance the offset.
  IoResult PeekAtLeastNInto(uint8_t* out, uint64_t n);

  // Unreads n bytes from the buffer. Moves offset back to |n| or less if
  // available buffer is smaller.
  void UnreadN(uint64_t n);

  BufferedSource* source() { return source_.get(); }

  ~BufReader();

 private:
  std::unique_ptr<BufferedSource> source_;
};

}  // namespace io

#endif  // IO_IO_BUF_READER_H_
