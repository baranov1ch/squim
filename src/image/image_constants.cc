#include "image/image_constants.h"

#include "glog/logging.h"

namespace image {

size_t GetBytesPerPixel(ColorScheme scheme) {
  switch (scheme) {
    case ColorScheme::kGrayScale:
      return 1;
    case ColorScheme::kGrayScaleAlpha:
      return 2;
    case ColorScheme::kRGB:
      return 3;
    case ColorScheme::kRGBA:
      return 4;
    default:
      DCHECK(false);
      return 0;
  }
}

}  // namespace image
