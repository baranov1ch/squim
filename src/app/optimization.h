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

#ifndef APP_OPTIMIZATION_H_
#define APP_OPTIMIZATION_H_

#include <memory>

#include "image/optimization/optimization_strategy.h"
#include "proto/image_optimizer.pb.h"

class Optimization {
 public:
  virtual std::unique_ptr<image::OptimizationStrategy>
  CreateOptimizationStrategy(const squim::ImageRequestPart_Meta& request) = 0;

  virtual ~Optimization() {}
};

class WebPOptimization : public Optimization {
 public:
  WebPOptimization();
  ~WebPOptimization() override;

  std::unique_ptr<image::OptimizationStrategy> CreateOptimizationStrategy(
      const squim::ImageRequestPart_Meta& request) override;
};

#endif  // APP_OPTIMIZATION_H_
