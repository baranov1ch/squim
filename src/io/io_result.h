#ifndef IO_IO_RESULT_H_
#define IO_IO_RESULT_H_

#include <cstddef>
#include <cstdint>

namespace io {

enum class IoResultCode {
  kOk,
  kPending,
  kEof,
  kError,
  // TODO: more errors.
};

class IoResult {
 public:
  static IoResult Pending();
  static IoResult Read(size_t n);
  static IoResult Eof();
  static IoResult Error();

  bool ok() const { return code_ == IoResultCode::kOk; }
  bool pending() const { return code_ == IoResultCode::kPending; }
  size_t n() const { return n_; }
  bool eof() const { return code_ == IoResultCode::kEof; }
  bool error() const { return code_ == IoResultCode::kError; }
  IoResultCode code() const { return code_; }

 private:
  IoResult(IoResultCode code);
  IoResult(size_t nread);

  IoResultCode code_;
  size_t n_;
};

}  // namespace io

#endif  // IO_IO_RESULT_H_
