#ifndef IMAGE_RESULT_H_
#define IMAGE_RESULT_H_

#include <ostream>
#include <string>

#include "io/io_result.h"

namespace image {

class Result {
 public:
  // TODO: macro generator.
  enum class Code {
    kOk,
    kImageTooLarge,
    kImageTooSmall,
    // TODO: separate read/write pend.
    kPending,
    kDecodeError,
    kErrorStart = kDecodeError,
    kEncodeError,
    kUnsupportedFormat,
    kUnexpectedEof,
    kIoErrorOther,
    kDunnoHowToEncode,
    kReadFrameError,
    kWriteFrameError,
    kFailed,
  };

  static Result Error(Code code);
  static Result Error(Code code, std::string custom_message);
  static Result Finish(Code code);
  static Result Finish(Code code, std::string custom_message);
  static Result Pending();
  static Result FromIoResult(io::IoResult io_result, bool eof_ok);
  static Result Ok();

  static const char* CodeToString(Code code);

  bool ok() const { return code_ == Code::kOk; }
  bool pending() const { return code_ == Code::kPending; }
  bool error() const { return code_ >= Code::kErrorStart; }
  bool finished() const { return finished_; }
  const std::string& custom_message() const { return custom_message_; }
  Code code() const { return code_; }

 private:
  Result(Code code, std::string custom_message, bool finished);

  Code code_;
  std::string custom_message_;
  bool finished_;
};

std::ostream& operator<<(std::ostream& os, Result::Code code);

}  // namespace image

#endif  // IMAGE_RESULT_H_
