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

#include "image/image_constants.h"

#include "base/logging.h"

namespace image {

size_t GetBytesPerPixel(ColorScheme scheme) {
  switch (scheme) {
    case ColorScheme::kGrayScale:
      return 1;
    case ColorScheme::kGrayScaleAlpha:
      return 2;
    case ColorScheme::kRGB:
      return 3;
    case ColorScheme::kRGBA:
      return 4;
    default:
      NOTREACHED();
      return 0;
  }
}

}  // namespace image
