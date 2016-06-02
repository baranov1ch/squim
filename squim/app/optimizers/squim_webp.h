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

#ifndef SQUIM_APP_OPTIMIZERS_SQUIM_WEBP_H_
#define SQUIM_APP_OPTIMIZERS_SQUIM_WEBP_H_

#include "proto/image_optimizer.pb.h"
#include "squim/image/optimization/layered_adjuster.h"

class SquimWebP : public image::LayeredAdjuster::Layer {
 public:
  SquimWebP(const squim::ImageRequestPart_Meta& request);

  bool ShouldWaitForMetadata() override;
  void AdjustWebPEncoderParams(image::WebPEncoder::Params* params) override;

 private:
  squim::ImageRequestPart_Meta request_;
};

#endif  // SQUIM_APP_OPTIMIZERS_SQUIM_WEBP_H_
