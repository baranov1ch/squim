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

#include "squim/image/optimization/codec_configurator.h"

namespace image {

CodecConfigurator::CodecConfigurator() {}

CodecConfigurator::~CodecConfigurator() {}

GifDecoder::Params CodecConfigurator::GetGifDecoderParams() {
  return GifDecoder::Params::Default();
}

JpegDecoder::Params CodecConfigurator::GetJpegDecoderParams() {
  return JpegDecoder::Params::Default();
}

PngDecoder::Params CodecConfigurator::GetPngDecoderParams() {
  return PngDecoder::Params::Default();
}

WebPDecoder::Params CodecConfigurator::GetWebPDecoderParams() {
  return WebPDecoder::Params::Default();
}

WebPEncoder::Params CodecConfigurator::GetWebPEncoderParams() {
  return WebPEncoder::Params::Default();
}

}  // namespace image
