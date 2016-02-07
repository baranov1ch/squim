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

#ifndef SQUIM_IMAGE_OPTIMIZATION_CONVERT_TO_WEBP_STRATEGY_H_
#define SQUIM_IMAGE_OPTIMIZATION_CONVERT_TO_WEBP_STRATEGY_H_

#include "squim/base/make_noncopyable.h"
#include "squim/image/optimization/codec_aware_strategy.h"

namespace image {

class ImageCodecFactory;

class ConvertToWebPStrategy : public CodecAwareStrategy {
  MAKE_NONCOPYABLE(ConvertToWebPStrategy);

 public:
  using CodecFactoryBuilder =
      std::function<std::unique_ptr<ImageCodecFactory>(CodecConfigurator*)>;
  ConvertToWebPStrategy(CodecFactoryBuilder codec_factory_builder);
  ~ConvertToWebPStrategy() override;

  // CodecAwareStrategy implementation:
  Result ShouldEvenBother() override;
  Result CreateImageReader(ImageType image_type,
                           std::unique_ptr<io::BufReader> src,
                           std::unique_ptr<ImageReader>* reader) override;
  Result CreateImageWriter(std::unique_ptr<io::VectorWriter> dest,
                           ImageReader* reader,
                           std::unique_ptr<ImageWriter>* writer) override;
  Result AdjustImageReaderAfterInfoReady(
      std::unique_ptr<ImageReader>* reader) override;
  bool ShouldWaitForMetadata() override;
  GifDecoder::Params GetGifDecoderParams() override;
  JpegDecoder::Params GetJpegDecoderParams() override;
  PngDecoder::Params GetPngDecoderParams() override;
  WebPDecoder::Params GetWebPDecoderParams() override;
  WebPEncoder::Params GetWebPEncoderParams() override;

 private:
  std::unique_ptr<ImageCodecFactory> codec_factory_;

  // Allow mixed lossy/lossless compression for multiframe images.
  bool allow_mixed_ = false;
};

}  // namespace image

#endif  // SQUIM_IMAGE_OPTIMIZATION_CONVERT_TO_WEBP_STRATEGY_H_
