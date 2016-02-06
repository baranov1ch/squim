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

#ifndef SQUIM_IMAGE_IMAGE_CONSTANTS_H_
#define SQUIM_IMAGE_IMAGE_CONSTANTS_H_

#include <cstdlib>

namespace image {

enum class ColorScheme {
  kGrayScale,
  kGrayScaleAlpha,
  kRGB,
  kRGBA,
  kYUV,
  kYUVA,
  kUnknown,
};

enum class ImageType {
  kJpeg,
  kPng,
  kGif,
  kWebP,
  kUnknown,
};

size_t GetBytesPerPixel(ColorScheme scheme);

}  // namespace image

#endif  // SQUIM_IMAGE_IMAGE_CONSTANTS_H_
