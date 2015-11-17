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

enum class ImageType {
  kJpeg,
  kPng,
  kGif,
  kWebP,
  kUnknown,
};

}  // namespace image

#endif  // IMAGE_IMAGE_CONSTANTS_H_
