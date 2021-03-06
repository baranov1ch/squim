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

#ifndef SQUIM_IMAGE_CODECS_DECODE_PARAMS_H_
#define SQUIM_IMAGE_CODECS_DECODE_PARAMS_H_

#include <set>

#include "squim/image/image_constants.h"

namespace image {

struct DecodeParams {
  bool color_scheme_allowed(ColorScheme scheme) const {
    return allowed_color_schemes.find(scheme) != allowed_color_schemes.end();
  }
  std::set<ColorScheme> allowed_color_schemes;
};

}  // namespace image

#endif  // SQUIM_IMAGE_CODECS_DECODE_PARAMS_H_
