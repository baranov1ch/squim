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

#include "squim/image/image_frame.h"

namespace image {

ImageFrame::ImageFrame() {}

ImageFrame::~ImageFrame() {}

void ImageFrame::Init() {
  DCHECK(!data_);
  DCHECK(color_scheme_ != ColorScheme::kUnknown);
  data_.reset(new uint8_t[height_ * stride()]);
}

}  // namespace image
