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

#ifndef SQUIM_IMAGE_CODECS_GIF_GIF_IMAGE_H_
#define SQUIM_IMAGE_CODECS_GIF_GIF_IMAGE_H_

#include <array>
#include <memory>
#include <vector>

#include "squim/image/image_metadata.h"
#include "squim/image/pixel.h"
#include "squim/image/result.h"

namespace image {

class GifImage {
 public:
  enum class DisposalMethod {
    kNotSpecified,
    kKeep,
    kOverwriteBgcolor,
    kOverwritePrevious,
  };

  static const size_t kInifiniteLoop = 0xFFFFFFFF;
  static const size_t kNoTransparentPixel = 0xFFFFFFFF;
  static const size_t kNoBackgroundColor = 0xFFFFFFFF;

  class ColorTable {
   public:
    static const int kNumBytesPerEntry = 3;

    ColorTable(size_t table_size);
    ~ColorTable();

    size_t expected_size() const { return expected_size_; }
    size_t size() const { return table_.size(); }

    void AddColor(uint8_t r, uint8_t g, uint8_t b);
    const RGBPixel GetColor(size_t idx) const;

   private:
    std::vector<std::array<uint8_t, kNumBytesPerEntry>> table_;
    size_t expected_size_;
  };

  class Frame {
   public:
    class Parser;
    class Builder;

    Frame();
    ~Frame();

    const ColorTable* GetColorTable() const;
    uint8_t GetPixel(uint16_t x, uint16_t y) const;

    uint16_t width() const { return width_; }
    uint16_t height() const { return height_; }
    uint16_t x_offset() const { return x_offset_; }
    uint16_t y_offset() const { return y_offset_; }
    size_t transparent_pixel() const { return transparent_pixel_; }
    size_t duration() const { return duration_; }
    DisposalMethod disposal_method() const { return disposal_method_; }
    bool is_progressive() const { return is_progressive_; }

   private:
    void SetRow(uint16_t nrow, uint8_t* row_data);

    uint16_t width_ = 0;
    uint16_t height_ = 0;
    uint16_t x_offset_ = 0;
    uint16_t y_offset_ = 0;
    size_t transparent_pixel_ = kNoTransparentPixel;
    size_t duration_ = 0;
    DisposalMethod disposal_method_ = DisposalMethod::kNotSpecified;
    bool is_progressive_ = false;

    std::unique_ptr<uint8_t[]> data_;

    std::unique_ptr<ColorTable> local_color_table_;
    const ColorTable* global_color_table_ = nullptr;
  };

  class Parser;
  class Builder;

  using FrameList = std::vector<std::unique_ptr<Frame>>;

  GifImage();
  ~GifImage();

  const ColorTable* global_color_table() const {
    return global_color_table_.get();
  }

  int version() const { return version_; }
  uint16_t screen_width() const { return screen_width_; }
  uint16_t screen_height() const { return screen_height_; }
  uint8_t color_resolution() const { return color_resolution_; }

  size_t background_color_index() const { return background_color_index_; }
  size_t loop_count() const { return loop_count_; }

  const FrameList& frames() const { return frames_; }
  ImageMetadata* GetMetadata() { return &metadata_; }

 private:
  FrameList frames_;
  std::unique_ptr<ColorTable> global_color_table_;
  uint16_t screen_width_ = 0;
  uint16_t screen_height_ = 0;
  size_t loop_count_ = 0;
  int version_ = 0;
  uint8_t color_resolution_ = 0;
  size_t background_color_index_ = kNoBackgroundColor;
  ImageMetadata metadata_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_CODECS_GIF_GIF_IMAGE_H_
