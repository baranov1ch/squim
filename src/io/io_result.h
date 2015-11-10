#ifndef IO_IO_RESULT_H_
#define IO_IO_RESULT_H_

#include <stdint.h>

namespace io {

enum class IoResultCode {
  kOk,
  kPending,
  kEOF,
  kError,
  // TODO: more errors.
};

class IoResult {
 public:
  static IoResult Pending();
  static IoResult Read(uint64_t n);
  static IoResult EoF();
  static IoResult Error();

  bool ok() const { return code_ == IoResultCode::kOk; }
  bool pending() const { return code_ == IoResultCode::kPending; }
  uint64_t nread() const { return nread_; }
  bool eof() const { return code_ == IoResultCode::kEOF; }
  bool error() const { return code_ == IoResultCode::kError; }
  IoResultCode code() const { return code_; }

 private:
  IoResult(IoResultCode code);
  IoResult(uint64_t nread);

  IoResultCode code_;
  uint64_t nread_;
};

}  // namespace io

#endif  // IO_IO_RESULT_H_
