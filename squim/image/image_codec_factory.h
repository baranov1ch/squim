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

#ifndef SQUIM_IMAGE_IMAGE_CODEC_FACTORY_H_
#define SQUIM_IMAGE_IMAGE_CODEC_FACTORY_H_

#include <memory>

#include "squim/image/image_constants.h"

namespace io {
class BufReader;
class VectorWriter;
}

namespace image {

class ImageDecoder;
class ImageEncoder;

// Abstract codec factory interface.
class ImageCodecFactory {
 public:
  virtual std::unique_ptr<ImageDecoder> CreateDecoder(
      ImageType type,
      std::unique_ptr<io::BufReader> reader) = 0;
  virtual std::unique_ptr<ImageEncoder> CreateEncoder(
      ImageType type,
      std::unique_ptr<io::VectorWriter> writer) = 0;

  virtual ~ImageCodecFactory() {}
};

}  // namespace image

#endif  // SQUIM_IMAGE_IMAGE_CODEC_FACTORY_H_
