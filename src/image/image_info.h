#ifndef IMAGE_IMAGE_INFO_H_
#define IMAGE_IMAGE_INFO_H_

namespace image {

struct ImageInfo {
  ColorScheme color_scheme;
  uint32_t width;
  uint32_t height;
  uint64_t size;
};

}  // namespace image

#endif  // IMAGE_IMAGE_INFO_H_
