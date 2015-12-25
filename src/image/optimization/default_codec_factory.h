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

#ifndef IMAGE_OPTIMIZATION_DEFAULT_CODEC_FACTORY_H_
#define IMAGE_OPTIMIZATION_DEFAULT_CODEC_FACTORY_H_

#include "image/optimization/codec_factory_with_configurator.h"

namespace image {

class DefaultCodecFactory : public CodecFactoryWithConfigurator {
 public:
  DefaultCodecFactory(CodecConfigurator* configurator);
  ~DefaultCodecFactory() override;

  std::unique_ptr<ImageDecoder> CreateDecoder(
      ImageType type,
      std::unique_ptr<io::BufReader> reader) override;
  std::unique_ptr<ImageEncoder> CreateEncoder(
      ImageType type,
      std::unique_ptr<io::VectorWriter> writer) override;
};

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_DEFAULT_CODEC_FACTORY_H_
