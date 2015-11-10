#ifndef IMAGE_IMAGE_FRAME_H_
#define IMAGE_IMAGE_FRAME_H_

#include <memory>
#include <stdint.h>

namespace image {

class ImageFrame {
  MAKE_NONCOPYABLE(ImageFrame);
 public:
  ImageFrame();
  ~ImageFrame();

  uint32_t width() const;
  uint32_t height() const;
  uint32_t duration() const;
  ColorScheme color_scheme() const;
  bool has_alpha() const;
  size_t required_previous_frame_index() const;

 private: 
  uint8_t* GetPixelData(uint32_t x, uint32_t y);
  void SetPixelData(uint32_t x, uint32_t y, uint8_t* data);

  std::unique_ptr<char[]> data_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_FRAME_H_
