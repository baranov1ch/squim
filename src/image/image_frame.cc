#include "image/image_frame.h"

namespace image {

ImageFrame::ImageFrame() {}

ImageFrame::~ImageFrame() {}

uint32_t ImageFrame::stride() const {
  return width_ * bpp_;
}

bool ImageFrame::has_alpha() const {
  return color_scheme_ == ColorScheme::kRGBA ||
         color_scheme_ == ColorScheme::kGrayScaleAlpha;
}

void ImageFrame::Init(uint32_t width,
                      uint32_t height,
                      ColorScheme color_scheme) {
  color_scheme_ = color_scheme;
  width_ = width;
  height_ = height;
  bpp_ = GetBytesPerPixel(color_scheme_);
  data_.reset(new uint8_t[height_ * stride()]);
}

}  // namespace image
