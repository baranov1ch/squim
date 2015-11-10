#ifndef IO_IO_ERROR_H_
#define IO_IO_ERROR_H_

#include <stdint.h>

namespace io {

enum class IoErrorCode {
  kOk,
  kPending,
  kEOF,
  kError,
  // TODO: more errors.
};

class IoError {
 public:
  static IoError Pending();
  static IoError Read(uint64_t n);
  static IoError EOF();
  static IoError Error();

  bool ok() const { return code_ == IoErrorCode::kOk; }
  bool pending() const { return code_ == IoErrorCode::kPending; }
  uint64_t nread() const { return nread_; }
  bool eof() const { return code_ == IoErrorCode::kEOF; }
  bool error() const { return code_ == IoErrorCode::kError; }
  IoErrorCode code() const { return code_; }

 private:
  IoError(IoErrorCode code);
  IoError(uint64_t nread);

  IoErrorCode code_;
  uint64_t nread_;
};

}  // namespace io

#endif  // IO_IO_ERROR_H_
