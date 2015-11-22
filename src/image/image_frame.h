#ifndef IMAGE_IMAGE_FRAME_H_
#define IMAGE_IMAGE_FRAME_H_

#include <memory>
#include <cstdint>

#include "base/make_noncopyable.h"
#include "image/image_constants.h"

namespace image {

class ImageFrame {
  MAKE_NONCOPYABLE(ImageFrame);

 public:
  ImageFrame() {}
  ~ImageFrame() {}

  uint32_t width() const { return 0; }
  uint32_t height() const { return 0; }
  uint32_t duration() const { return 0; }
  ColorScheme color_scheme() const { return ColorScheme::kRGBA; }
  bool has_alpha() const { return false; }
  size_t required_previous_frame_index() const { return 0; }
  ImageType image_type() const { return ImageType::kUnknown; }

 private:
  uint8_t* GetPixelData(uint32_t x, uint32_t y) { return nullptr; }
  void SetPixelData(uint32_t x, uint32_t y, uint8_t* data) {}

  std::unique_ptr<char[]> data_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_FRAME_H_
