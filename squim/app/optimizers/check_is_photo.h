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

#ifndef SQUIM_APP_OPTIMIZERS_CHECK_IS_PHOTO_H_
#define SQUIM_APP_OPTIMIZERS_CHECK_IS_PHOTO_H_

#include "proto/image_optimizer.pb.h"
#include "squim/image/codecs/webp_encoder.h"
#include "squim/image/optimization/layered_adjuster.h"

// Checks photo-like metric for PNG images and sets up encoder to lossless
// mode if image does not look like photo (since lossy webp does not handle)
// well sharp-edged images.
class CheckIsPhoto : public image::LayeredAdjuster::Layer {
 public:
  CheckIsPhoto(const squim::ImageRequestPart_Meta& request);

  image::Result AdjustReaderAfterInfoReady(
      std::unique_ptr<image::ImageReader>* reader) override;
  void AdjustWebPEncoderParams(image::WebPEncoder::Params* params) override;
};

#endif  // SQUIM_APP_OPTIMIZERS_CHECK_IS_PHOTO_H_
