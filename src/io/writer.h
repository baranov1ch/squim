#ifndef IO_WRITER_H_
#define IO_WRITER_H_

#include "io/io_result.h"

namespace io {

class Chunk;

class Writer {
 public:
  virtual IoResult Write(Chunk* chunk) = 0;

  virtual ~Writer() {}
};

}  // namespace io

#endif  // IO_WRITER_H_
