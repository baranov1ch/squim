#ifndef IMAGE_IMAGE_CONSTANTS_H_
#define IMAGE_IMAGE_CONSTANTS_H_

#include <cstdlib>

namespace image {

enum class ColorScheme {
  kGrayScale,
  kGrayScaleAlpha,
  kRGB,
  kRGBA,
  kYUV,
  kCMYK,
  kUnknown,
};

enum class ImageType {
  kJpeg,
  kPng,
  kGif,
  kWebP,
  kUnknown,
};

size_t GetBytesPerPixel(ColorScheme scheme);

}  // namespace image

#endif  // IMAGE_IMAGE_CONSTANTS_H_
