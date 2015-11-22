#ifndef IMAGE_IMAGE_INFO_H_
#define IMAGE_IMAGE_INFO_H_

#include <cstdint>

#include "image/image_constants.h"

namespace image {

struct ImageInfo {
  ColorScheme color_scheme;
  uint32_t width;
  uint32_t height;
  uint64_t size;
  ImageType type;
  bool multiframe;
};

}  // namespace image

#endif  // IMAGE_IMAGE_INFO_H_
