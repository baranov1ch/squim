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

#ifndef IMAGE_OPTIMIZATION_CODEC_FACTORY_WITH_CONFIGURATOR_H_
#define IMAGE_OPTIMIZATION_CODEC_FACTORY_WITH_CONFIGURATOR_H_

#include "image/codec_factory.h"

namespace image {

class CodecFactoryWithConfigurator : public ImageCodecFactory {
 protected:
  CodecFactoryWithConfigurator(CodecConfigurator* configurator);

  CodecConfigurator* configurator() { return configurator_; }

 private:
  CodecConfigurator* configurator_;
};

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_CODEC_FACTORY_WITH_CONFIGURATOR_H_
