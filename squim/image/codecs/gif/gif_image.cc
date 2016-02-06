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

#include "squim/image/codecs/gif/gif_image.h"

#include <cstring>

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"

namespace image {

GifImage::ColorTable::ColorTable(size_t table_size)
    : expected_size_(table_size) {}

GifImage::ColorTable::~ColorTable() {}

void GifImage::ColorTable::AddColor(uint8_t r, uint8_t g, uint8_t b) {
  if (table_.empty())
    table_.reserve(expected_size_);

  std::array<uint8_t, kNumBytesPerEntry> elt{{r, g, b}};
  table_.push_back(std::move(elt));
}

const RGBPixel GifImage::ColorTable::GetColor(size_t idx) const {
  DCHECK_GT(table_.size(), idx);
  const auto& color_data = table_[idx];
  return RGBPixel(const_cast<uint8_t*>(&color_data[0]));
}

GifImage::Frame::Frame() {}

GifImage::Frame::~Frame() {}

void GifImage::Frame::SetRow(uint16_t nrow, uint8_t* row_data) {
  std::memcpy(data_.get() + (nrow * width_), row_data, width_);
}

const GifImage::ColorTable* GifImage::Frame::GetColorTable() const {
  if (local_color_table_)
    return local_color_table_.get();
  return global_color_table_;
}

uint8_t GifImage::Frame::GetPixel(uint16_t x, uint16_t y) const {
  DCHECK_GT(width_, x);
  DCHECK_GT(height_, y);
  return data_[width_ * y + x];
}

GifImage::GifImage() {}

GifImage::~GifImage() {}

}  // namespace image
