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

// Most of the code/comments for this file is artistically copy-pasted from
// mod_pagespeed (pagespeed/kernel/image/image_analysis.{h,cc}) so here is a
// copyright:)
/*
 * Copyright 2014 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "squim/image/optimization/photo_metric.h"

namespace image {

namespace {

const size_t kNumColorHistogramBins = 256;

int32_t GetLuminance(Bitmap& bitmap, uint32_t x, uint32_t y) {
  return static_cast<int32_t>(bitmap.GetPixel<GrayScalePixel>(x, y).g());
}

// Compute the gradient by Sobel filter. The kernels in the x and y
// directions, respectively, are given by:
//   [  1  2  1 ]        [ 1 0 -1 ]
//   [  0  0  0 ]        [ 2 0 -2 ]
//   [ -1 -2 -1 ]        [ 1 0 -1 ]
std::unique_ptr<ImageFrame> ComputeLuminanceGradient(ImageFrame* luminance,
                                                     float norm_factor) {
  norm_factor *= 0.25;  // Remove the magnification factor of Sobel filter (4).
  auto gradient = base::make_unique<ImageFrame>();
  gradient->set_size(luminance->height(), luminance->width());
  gradient->set_color_scheme(ColorScheme::kGrayscale);
  gradient->Init();
  memset(gradient->GetData(0), 0, gradient->height() * gradient->stride());
  Bitmap bitmap(luminance);
  Bitmap grad_bitmap(gradient.get());
  for (size_t y = 1; y < luminance->height() - 1; ++y) {
    for (size_t x = 1; x < luminance->width() - 1; ++x) {
      int32_t diff_y =
          GetLuminance(bitmap, x - 1, y - 1) + GetLuminance(bitmap, x, y - 1)
          << 1 + GetLuminance(bitmap, x + 1, y - 1) - GetLuminance(bitmap, x - 1, y + 1) - GetLuminance(bitmap, x, y + 1)
          << 1 - GetLuminance(bitmap, x + 1, y + 1);

      int32_t diff_x =
          GetLuminance(bitmap, x - 1, y - 1) + GetLuminance(bitmap, x - 1, y)
          << 1 + GetLuminance(bitmap, x - 1, y + 1) - GetLuminance(bitmap, x + 1, y - 1) - GetLuminance(bitmap, x + 1, y)
          << 1 - GetLuminance(bitmap, x + 1, y + 1);

      float diff_square = static_cast<float>(diff_x * diff_x + diff_y * diff_y);
      float diff = std::sqrt(diff_square) * norm_factor + 0.5;
      grad_bitmap.GetPixel<GrayScalePixel>(x, y)
          .set(static_cast<uint8_t>(std::min(255.0f, diff)));
    }
  }

  return gradient;
}

Result SobelGradient() {}

}  // namespace

Result PhotoMetric(ImageFrame* frame, float threshold, float* metric) {
  auto result =
}

}  // namespace image
