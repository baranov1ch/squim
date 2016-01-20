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

#ifndef IMAGE_IMAGE_INFO_H_
#define IMAGE_IMAGE_INFO_H_

#include <array>
#include <cstdint>

#include "image/image_constants.h"

namespace image {

struct ImageInfo {
  uint32_t width = 0;
  uint32_t height = 0;
  uint64_t size = 0;
  ImageType type = ImageType::kUnknown;
  bool multiframe = false;
  size_t loop_count = 0;
  std::array<uint8_t, 4> bg_color;
};

}  // namespace image

#endif  // IMAGE_IMAGE_INFO_H_
