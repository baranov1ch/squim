#ifndef IMAGE_RESULT_H_
#define IMAGE_RESULT_H_

#include <string>

namespace image {

enum class ErrorCode {
  kOk,
  kDecodeError,
  kUnsupportedFormat,
};

class Result {
 public:
  static Result Finish(ErrorCode code, std::string message);
  static Result Pending();
  static Result Ok();

  bool ok() const;
  bool pending() const;
  bool error() const;
  bool progressed() const;
  bool finished() const;
  bool meta() const;

 private:
  ErrorCode code_;
  std::string custom_message_;
};

}  // namespace image

#endif  // IMAGE_RESULT_H_
