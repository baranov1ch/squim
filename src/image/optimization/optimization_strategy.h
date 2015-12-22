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

#ifndef IMAGE_OPIMIZATION_OPTIMIZATION_STRATEGY_H_
#define IMAGE_OPIMIZATION_OPTIMIZATION_STRATEGY_H_

#include <memory>

#include "image/result.h"

namespace io {
class BufReader;
class VectorWriter;
}

namespace image {

class ImageReader;
class ImageWriter;

class OptimizationStrategy {
 public:
  virtual Result ShouldEvenBother() = 0;
  virtual Result CreateImageReader(ImageType image_type,
                                   std::unique_ptr<io::BufReader> src,
                                   std::unique_ptr<ImageReader>* reader) = 0;
  virtual Result CreateImageWriter(std::unique_ptr<io::VectorWriter> dest,
                                   ImageReader* reader,
                                   std::unique_ptr<ImageWriter>* writer) = 0;
  virtual Result AdjustImageReaderAfterInfoReady(
      std::unique_ptr<ImageReader>* reader) = 0;
  virtual bool ShouldWaitForMetadata() = 0;

  virtual ~OptimizationStrategy() {}
};

}  // namespace image

#endif  // IMAGE_OPIMIZATION_OPTIMIZATION_STRATEGY_H_
