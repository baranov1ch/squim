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

#ifndef SQUIM_IMAGE_OPTIMIZATION_CODEC_AWARE_STRATEGY_H_
#define SQUIM_IMAGE_OPTIMIZATION_CODEC_AWARE_STRATEGY_H_

#include "squim/image/optimization/codec_configurator.h"
#include "squim/image/optimization/optimization_strategy.h"

namespace image {

class CodecAwareStrategy : public OptimizationStrategy,
                           public CodecConfigurator {};

}  // namespace image

#endif  // SQUIM_IMAGE_OPTIMIZATION_CODEC_AWARE_STRATEGY_H_
