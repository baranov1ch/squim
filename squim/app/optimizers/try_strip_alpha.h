/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#ifndef SQUIM_APP_OPTIMIZERS_TRY_STRIP_ALPHA_H_
#define SQUIM_APP_OPTIMIZERS_TRY_STRIP_ALPHA_H_

#include "squim/image/optimization/layered_adjuster.h"

// Tries to strip alpha channel from the image if it is completely opaque.
// Works only for PNG images, since jpegs have no alpha and gifs are always
// encoded into webp with alpha. I future may be enabled for webp images too.
class TryStripAlpha : public image::LayeredAdjuster::Layer {
 public:
  image::Result AdjustWriter(
      image::ImageReader* reader,
      std::unique_ptr<image::ImageWriter>* writer) override;
};

#endif  // SQUIM_APP_OPTIMIZERS_TRY_STRIP_ALPHA_H_
