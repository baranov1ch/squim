#ifndef IO_READER_H_
#define IO_READER_H_

#include "io/io_result.h"

namespace io {

class Chunk;

class Reader {
 public:
  virtual IoResult Read(Chunk* chunk) = 0;

  virtual ~Reader() {}
};

}  // namespace io

#endif  // IO_READER_H_
