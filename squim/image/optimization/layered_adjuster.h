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

#ifndef SQUIM_IMAGE_OPIMIZATION_LAYERED_ADJUSTER_H_
#define SQUIM_IMAGE_OPIMIZATION_LAYERED_ADJUSTER_H_

#include <memory>

#include "squim/image/optimization/root_strategy.h"

namespace image {

class LayeredAdjuster : public RootStrategy::Adjuster {
 public:
  class Layer : public RootStrategy::Adjuster {
   public:
    Result ShouldEvenBother() override;
    Result AdjustReader(ImageType image_type,
                        std::unique_ptr<ImageReader>* reader) override;
    Result AdjustReaderAfterInfoReady(
        std::unique_ptr<ImageReader>* reader) override;
    Result AdjustWriter(ImageReader* reader,
                        std::unique_ptr<ImageWriter>* writer) override;
    bool ShouldWaitForMetadata() override;
    void AdjustGifDecoderParams(GifDecoder::Params* params) override;
    void AdjustJpegDecoderParams(JpegDecoder::Params* params) override;
    void AdjustPngDecoderParams(PngDecoder::Params* params) override;
    void AdjustWebPDecoderParams(WebPDecoder::Params* params) override;
    void AdjustWebPEncoderParams(WebPEncoder::Params* params) override;
  };

  LayeredAdjuster(std::unique_ptr<Layer> impl,
                  std::unique_ptr<LayeredAdjuster> next);
  ~LayeredAdjuster() override;

  Result ShouldEvenBother() override;
  Result AdjustReader(ImageType image_type,
                      std::unique_ptr<ImageReader>* reader) override;
  Result AdjustReaderAfterInfoReady(
      std::unique_ptr<ImageReader>* reader) override;
  Result AdjustWriter(ImageReader* reader,
                      std::unique_ptr<ImageWriter>* writer) override;
  bool ShouldWaitForMetadata() override;
  void AdjustGifDecoderParams(GifDecoder::Params* params) override;
  void AdjustJpegDecoderParams(JpegDecoder::Params* params) override;
  void AdjustPngDecoderParams(PngDecoder::Params* params) override;
  void AdjustWebPDecoderParams(WebPDecoder::Params* params) override;
  void AdjustWebPEncoderParams(WebPEncoder::Params* params) override;

 private:
  std::unique_ptr<Layer> impl_;
  std::unique_ptr<LayeredAdjuster> next_;
};

class NullAdjuster : public LayeredAdjuster::Layer {};

}  // namespace image

#endif  // SQUIM_IMAGE_OPIMIZATION_LAYERED_ADJUSTER_H_
