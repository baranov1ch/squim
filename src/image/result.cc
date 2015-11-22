#include "image/result.h"

namespace image {

// static
Result Result::Ok() {
  return Result(Code::kOk, std::string(), false);
}

// static
Result Result::Pending() {
  return Result(Code::kPending, std::string(), false);
}

// static
Result Result::Error(Code code) {
  return Result(code, std::string(), false);
}

// static
Result Result::Finish(Code code) {
  return Result(code, std::string(), true);
}

// static
Result Result::FromIoResult(io::IoResult io_result, bool eof_ok) {
  if (io_result.ok())
    return Result::Ok();

  if (io_result.pending())
    return Result::Pending();

  if (io_result.eof()) {
    if (eof_ok) {
      return Result::Finish(Code::kOk);
    }
    return Result::Error(Code::kUnexpectedEof);
  }

  return Result::Error(Code::kIoErrorOther);
}

Result::Result(Code code, std::string custom_message, bool finished)
    : code_(code), custom_message_(custom_message), finished_(finished) {}

}  // namespace image
