#ifndef IMAGE_IMAGE_CONSTANTS_H_
#define IMAGE_IMAGE_CONSTANTS_H_

namespace image {

enum class ColorScheme {
  kGrayScale,
  kGrayScaleAlpha,
  kRGB,
  kRGBA,
  kYUV,
  kCMYK,
};

enum class ErrorCode {
  kOk,
  kDecodeError,
};

class Result {
 public:
  bool Ok() const;
  bool Pending() const;
  bool Error() const;
  bool Progressed() const;
  bool Finished() const;
  bool Meta() const;
 private:
  ErrorCode code_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_CONSTANTS_H_
