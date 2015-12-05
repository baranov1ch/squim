#ifndef IO_CLOSER_H_
#define IO_CLOSER_H_

#include "io/io_result.h"

namespace io {

class Closer {
 public:
  virtual IoResult Close() = 0;

  virtual ~Closer() {}
};

}  // namespace io

#endif  // IO_CLOSER_H_
