#include "io/io_result.h"

namespace io {

// static
IoResult IoResult::Pending() {
  return IoResult(IoResultCode::kPending);
}

// static
IoResult IoResult::Read(size_t nread) {
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

IoResult::IoResult(IoResultCode code) : code_(code), n_(0u) {}

IoResult::IoResult(size_t n) : code_(IoResultCode::kOk), n_(n) {}

}  // namespace io
