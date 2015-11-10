#include "io/io_error.h"

namespace io {

// static
IoError IoError::Pending() {
  return IoError(IoErrorCode::kPending);
}

// static
IoError IoError::Read(uint64_t nread) {
  return IoError(nread);
}

// static
IoError IoError::EOF() {
  return IoError(IoErrorCode::kEOF);
}

// static
IoError IoError::Error() {
  return IoError(IoErrorCode::kError);
}

IoError::IoError(IoErrorCode code) : code_(code), nread_(0u) {}

IoError::IoError(uint64_t nread) : code_(IoErrorCode::kOk), nread_(nread) {}

}  // namespace io
