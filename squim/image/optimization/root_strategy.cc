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

#include "squim/image/optimization/root_strategy.h"

#include "squim/image/image_codec_factory.h"
#include "squim/io/buf_reader.h"
#include "squim/io/writer.h"

namespace image {

RootStrategy::RootStrategy(CodecFactoryBuilder codec_factory_builder,
                           std::unique_ptr<CodecAwareStrategy> base_strategy,
                           std::unique_ptr<Adjuster> adjuster)
    : base_strategy_(std::move(base_strategy)), adjuster_(std::move(adjuster)) {
  codec_factory_ = codec_factory_builder(this);
  base_strategy_->SetCodecFactory(codec_factory_.get());
}

RootStrategy::~RootStrategy() {}

Result RootStrategy::ShouldEvenBother() {
  auto result = base_strategy_->ShouldEvenBother();
  if (!result.ok())
    return result;
  return adjuster_->ShouldEvenBother();
}

Result RootStrategy::CreateImageReader(ImageType image_type,
                                       std::unique_ptr<io::BufReader> src,
                                       std::unique_ptr<ImageReader>* reader) {
  auto result =
      base_strategy_->CreateImageReader(image_type, std::move(src), reader);
  if (!result.ok())
    return result;

  return adjuster_->AdjustReader(image_type, reader);
}

Result RootStrategy::CreateImageWriter(std::unique_ptr<io::VectorWriter> dest,
                                       ImageReader* reader,
                                       std::unique_ptr<ImageWriter>* writer) {
  auto result =
      base_strategy_->CreateImageWriter(std::move(dest), reader, writer);
  if (!result.ok())
    return result;

  return adjuster_->AdjustWriter(reader, writer);
}

Result RootStrategy::AdjustImageReaderAfterInfoReady(
    std::unique_ptr<ImageReader>* reader) {
  auto result = base_strategy_->AdjustImageReaderAfterInfoReady(reader);
  if (!result.ok())
    return result;

  return adjuster_->AdjustReaderAfterInfoReady(reader);
}

bool RootStrategy::ShouldWaitForMetadata() {
  bool should_wait = base_strategy_->ShouldWaitForMetadata();
  if (should_wait)
    return true;

  return adjuster_->ShouldWaitForMetadata();
}

GifDecoder::Params RootStrategy::GetGifDecoderParams() {
  auto params = base_strategy_->GetGifDecoderParams();
  adjuster_->AdjustGifDecoderParams(&params);
  return params;
}

JpegDecoder::Params RootStrategy::GetJpegDecoderParams() {
  auto params = base_strategy_->GetJpegDecoderParams();
  adjuster_->AdjustJpegDecoderParams(&params);
  return params;
}

PngDecoder::Params RootStrategy::GetPngDecoderParams() {
  auto params = base_strategy_->GetPngDecoderParams();
  adjuster_->AdjustPngDecoderParams(&params);
  return params;
}

WebPDecoder::Params RootStrategy::GetWebPDecoderParams() {
  auto params = base_strategy_->GetWebPDecoderParams();
  adjuster_->AdjustWebPDecoderParams(&params);
  return params;
}

WebPEncoder::Params RootStrategy::GetWebPEncoderParams() {
  auto params = base_strategy_->GetWebPEncoderParams();
  adjuster_->AdjustWebPEncoderParams(&params);
  return params;
}

void RootStrategy::SetCodecFactory(ImageCodecFactory* factory) {
  NOTREACHED();
}

}  // namespace image
