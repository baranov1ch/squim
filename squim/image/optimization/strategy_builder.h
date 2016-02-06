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

#ifndef SQUIM_IMAGE_OPIMIZATION_STRATEGY_BUILDER_H_
#define SQUIM_IMAGE_OPIMIZATION_STRATEGY_BUILDER_H_

#include <memory>

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/optimization/layered_adjuster.h"
#include "squim/image/optimization/root_strategy.h"

namespace image {

class StrategyBuilder {
 public:
  StrategyBuilder() {
    auto null_layer = base::make_unique<NullAdjuster>();
    layers_ = base::make_unique<LayeredAdjuster>(
        std::move(null_layer), std::unique_ptr<LayeredAdjuster>());
  }

  template <typename Strategy, typename... Args>
  StrategyBuilder& SetBaseStrategy(Args&&... args) {
    base_strategy_ = base::make_unique<Strategy>(std::forward<Args>(args)...);
    return *this;
  }

  template <typename Layer, typename... Args>
  StrategyBuilder& AddLayer(Args&&... args) {
    auto layer = base::make_unique<Layer>(std::forward<Args>(args)...);
    layers_ = base::make_unique<LayeredAdjuster>(std::move(layer),
                                                 std::move(layers_));
    return *this;
  }

  std::unique_ptr<OptimizationStrategy> Build() {
    CHECK(base_strategy_);
    return base::make_unique<RootStrategy>(std::move(base_strategy_),
                                           std::move(layers_));
  }

 private:
  std::unique_ptr<CodecAwareStrategy> base_strategy_;
  std::unique_ptr<LayeredAdjuster> layers_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_OPIMIZATION_STRATEGY_BUILDER_H_
