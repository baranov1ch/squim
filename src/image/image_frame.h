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
  const size_t kNoPreviousFrameIndex = 0xFFFFFFFF;

  enum class Status {
    kEmpty,
    kPartial,
    kComplete,
  };

  ImageFrame();
  ~ImageFrame();

  Status status() const { return status_; }
  void set_status(Status status) { status_ = status; }
  uint32_t width() const { return width_; }
  uint32_t height() const { return height_; }
  uint32_t stride() const;
  uint32_t duration() const { return duration_; }
  ColorScheme color_scheme() const { return color_scheme_; }
  bool has_alpha() const;
  size_t required_previous_frame_index() const {
    return required_previous_frame_index_;
  }

  size_t bpp() const { return bpp_; }

  uint8_t* GetPixel(uint32_t x, uint32_t y) {
    return data_.get() + (stride() * x + bpp_ * y);
  }

  const uint8_t* GetPixel(uint32_t x, uint32_t y) const {
    return GetPixel(x, y);
  }

  uint8_t* GetData(size_t offset) { return data_.get() + offset; }

  const uint8_t* GetData(size_t offset) const { return GetData(offset); }

  void Init(uint32_t width, uint32_t height, ColorScheme color_scheme);

 private:
  Status status_ = Status::kEmpty;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  uint32_t duration_ = 0;
  size_t bpp_ = 0;
  ColorScheme color_scheme_ = ColorScheme::kUnknown;
  size_t required_previous_frame_index_ = kNoPreviousFrameIndex;
  std::unique_ptr<uint8_t[]> data_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_FRAME_H_
