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

#ifndef IMAGE_OPTIMIZATION_CODEC_CONFIGURATOR_H_
#define IMAGE_OPTIMIZATION_CODEC_CONFIGURATOR_H_

#include "image/codecs/gif_decoder.h"
#include "image/codecs/jpeg_decoder.h"
#include "image/codecs/png_decoder.h"
#include "image/codecs/webp_decoder.h"
#include "image/codecs/webp_encoder.h"

namespace image {

class CodecConfigurator {
 public:
  CodecConfigurator();
  virtual ~CodecConfigurator();

  virtual GifDecoder::Params GetGifDecoderParams();
  virtual JpegDecoder::Params GetJpegDecoderParams();
  virtual PngDecoder::Params GetPngDecoderParams();
  virtual WebPDecoder::Params GetWebPDecoderParams();

  virtual WebPEncoder::Params GetWebPEncoderParams();
};

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_CODEC_CONFIGURATOR_H_
