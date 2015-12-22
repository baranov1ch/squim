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

#include "image/image_frame.h"

namespace image {

ImageFrame::ImageFrame() {}

ImageFrame::~ImageFrame() {}

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
