/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
  size_t bpp() const { return bpp_; }
  uint32_t stride() const { return width_ * bpp_; }
  uint32_t duration() const { return duration_; }
  ColorScheme color_scheme() const { return color_scheme_; }

  bool has_alpha() const {
    return color_scheme_ == ColorScheme::kRGBA ||
           color_scheme_ == ColorScheme::kGrayScaleAlpha ||
           color_scheme_ == ColorScheme::kYUVA;
  }

  bool is_grayscale() const {
    return color_scheme_ == ColorScheme::kGrayScale ||
           color_scheme_ == ColorScheme::kGrayScaleAlpha;
  }

  bool is_rgb() const {
    return color_scheme_ == ColorScheme::kRGB ||
           color_scheme_ == ColorScheme::kRGBA;
  }

  bool is_yuv() const {
    return color_scheme_ == ColorScheme::kYUV ||
           color_scheme_ == ColorScheme::kYUVA;
  }

  size_t required_previous_frame_index() const {
    return required_previous_frame_index_;
  }

  uint8_t* GetPixel(uint32_t x, uint32_t y) {
    return GetData(stride() * y + bpp() * x);
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

class Bitmap {
 public:
  Bitmap(ImageFrame* frame) : frame_(frame) {}

  template <typename Pixel>
  Pixel GetPixel(uint32_t x, uint32_t y) {
    DCHECK_EQ(Pixel::size(), frame_->bpp());
    return Pixel(frame_->GetPixel(x, y));
  }

  ImageFrame* frame_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_FRAME_H_
