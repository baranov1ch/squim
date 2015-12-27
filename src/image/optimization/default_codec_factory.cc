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

#include "image/optimization/default_codec_factory.h"

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/optimization/codec_configurator.h"
#include "io/buf_reader.h"
#include "io/writer.h"

namespace image {

// static
std::unique_ptr<ImageCodecFactory> DefaultCodecFactory::Builder(
    CodecConfigurator* configurator) {
  return base::make_unique<DefaultCodecFactory>(configurator);
}

DefaultCodecFactory::DefaultCodecFactory(CodecConfigurator* configurator)
    : CodecFactoryWithConfigurator(configurator) {}

DefaultCodecFactory::~DefaultCodecFactory() {}

std::unique_ptr<ImageDecoder> DefaultCodecFactory::CreateDecoder(
    ImageType type,
    std::unique_ptr<io::BufReader> reader) {
  switch (type) {
    case ImageType::kGif:
      return base::make_unique<GifDecoder>(
          configurator()->GetGifDecoderParams(), std::move(reader));
    case ImageType::kJpeg:
      return base::make_unique<JpegDecoder>(
          configurator()->GetJpegDecoderParams(), std::move(reader));
    case ImageType::kPng:
      return base::make_unique<PngDecoder>(
          configurator()->GetPngDecoderParams(), std::move(reader));
    case ImageType::kWebP:
      return base::make_unique<WebPDecoder>(
          configurator()->GetWebPDecoderParams(), std::move(reader));
    default:
      NOTREACHED();
      return std::unique_ptr<ImageDecoder>();
  }
}

std::unique_ptr<ImageEncoder> DefaultCodecFactory::CreateEncoder(
    ImageType type,
    std::unique_ptr<io::VectorWriter> writer) {
  switch (type) {
    case ImageType::kWebP:
      return base::make_unique<WebPEncoder>(
          configurator()->GetWebPEncoderParams(), std::move(writer));
    default:
      NOTREACHED();
      return std::unique_ptr<ImageEncoder>();
  }
}

}  // namespace image
