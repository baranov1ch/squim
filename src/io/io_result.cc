#include "io/io_result.h"

namespace io {

// static
IoResult IoResult::Pending() {
  return IoResult(IoResultCode::kPending);
}

// static
IoResult IoResult::Read(uint64_t nread) {
  return IoResult(nread);
}

// static
IoResult IoResult::Eof() {
  return IoResult(IoResultCode::kEof);
}

// static
IoResult IoResult::Error() {
  return IoResult(IoResultCode::kError);
}

IoResult::IoResult(IoResultCode code) : code_(code), nread_(0u) {}

IoResult::IoResult(uint64_t nread) : code_(IoResultCode::kOk), nread_(nread) {}

}  // namespace io
