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

#include "squim/app/optimization.h"

#include "squim/image/optimization/convert_to_webp_strategy.h"
#include "squim/image/optimization/strategy_builder.h"
#include "squim/image/optimization/default_codec_factory.h"

WebPOptimization::WebPOptimization() {}

WebPOptimization::~WebPOptimization() {}

std::unique_ptr<image::OptimizationStrategy>
WebPOptimization::CreateOptimizationStrategy(
    const squim::ImageRequestPart_Meta& request) {
  return image::StrategyBuilder()
      .SetBaseStrategy<image::ConvertToWebPStrategy>(
          image::DefaultCodecFactory::Builder)
      //.AddLayer<SquimWebP>(meta)
      //.AddLayer<TryStripAlpha>(meta)
      //.AddLayer<CheckIsPhoto>(meta)
      .Build();
}
