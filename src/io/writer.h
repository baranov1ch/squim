#ifndef IO_WRITER_H_
#define IO_WRITER_H_

#include "io/chunk.h"
#include "io/io_result.h"

namespace io {

class Writer {
 public:
  virtual IoResult Write(Chunk* chunk) = 0;

  virtual ~Writer() {}
};

class VectorWriter {
 public:
  virtual IoResult WriteV(ChunkList chunks) = 0;

  virtual ~VectorWriter() {}
};

}  // namespace io

#endif  // IO_WRITER_H_
