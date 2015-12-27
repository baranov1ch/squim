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

#ifndef IMAGE_OPIMIZATION_ROOT_STRATEGY_H_
#define IMAGE_OPIMIZATION_ROOT_STRATEGY_H_

#include <memory>

#include "image/optimization/codec_aware_strategy.h"

namespace image {

class RootStrategy : public CodecAwareStrategy {
 public:
  class Adjuster {
   public:
    virtual Result ShouldEvenBother() = 0;
    virtual Result AdjustReader(ImageType image_type,
                                std::unique_ptr<ImageReader>* reader) = 0;
    virtual Result AdjustReaderAfterInfoReady(
        std::unique_ptr<ImageReader>* reader) = 0;
    virtual Result AdjustWriter(ImageReader* reader,
                                std::unique_ptr<ImageWriter>* writer) = 0;
    virtual bool ShouldWaitForMetadata() = 0;
    virtual void AdjustGifDecoderParams(GifDecoder::Params* params) = 0;
    virtual void AdjustJpegDecoderParams(JpegDecoder::Params* params) = 0;
    virtual void AdjustPngDecoderParams(PngDecoder::Params* params) = 0;
    virtual void AdjustWebPDecoderParams(WebPDecoder::Params* params) = 0;
    virtual void AdjustWebPEncoderParams(WebPEncoder::Params* params) = 0;

    virtual ~Adjuster() {}
  };

  RootStrategy(std::unique_ptr<CodecAwareStrategy> base_strategy,
               std::unique_ptr<Adjuster> adjuster);
  ~RootStrategy() override;

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
  std::unique_ptr<CodecAwareStrategy> base_strategy_;
  std::unique_ptr<Adjuster> adjuster_;
};

}  // namespace image

#endif  // IMAGE_OPIMIZATION_ROOT_STRATEGY_H_
