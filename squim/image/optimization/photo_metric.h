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

#ifndef SQUIM_IMAGE_OPTIMIZATION_PHOTO_METRIC_H_
#define SQUIM_IMAGE_OPTIMIZATION_PHOTO_METRIC_H_

namespace image {

// Threshold for histogram. The histogram bins with values less than
// (max_hist_bin * kHistogramThreshold) will be ignored in computing
// the photo metric. Values of 0.005, 0.01, and 0.02 have been tried and
// the best one is 0.01.
const float kDefaultHistogramThreshold = 0.01;

// Returns the photographic metric. Photos will have large metric values, while
// computer generated graphics, especially those consisting of only a few colors
// or slowly changing colors will have small values. Graphics usually
// have tall and sharp peaks in their gradient histogram, while photo peaks are
// short and wide. Thus, the width of the widest peak in the color histogram
// can be used as the metric for photo-likeness. The metric can have values
// between 1 and 256, and the recommended threshold for separating graphics
// and photo is around 16.
Result PhotoMetric(ImageFrame* frame, float threshold, float* metric);

}  // namespace image

#endif  // SQUIM_IMAGE_OPTIMIZATION_PHOTO_METRIC_H_
