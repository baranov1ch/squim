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

#include "image/optimization/layered_adjuster.h"

namespace image {

LayeredAdjuster::LayeredAdjuster(std::unique_ptr<Layer> impl,
                                 std::unique_ptr<LayeredAdjuster> next)
    : impl_(std::move(impl)), next_(std::move(next)) {}

LayeredAdjuster::~LayeredAdjuster() {}

Result LayeredAdjuster::ShouldEvenBother() {
  auto result = impl_->ShouldEvenBother();
  if (!result.ok() || !next_)
    return result;
  return next_->ShouldEvenBother();
}

Result LayeredAdjuster::AdjustReader(ImageType image_type,
                                     std::unique_ptr<ImageReader>* reader) {
  auto result = impl_->AdjustReader(image_type, reader);
  if (!result.ok() || !next_)
    return result;
  return next_->AdjustReader(image_type, reader);
}

Result LayeredAdjuster::AdjustReaderAfterInfoReady(
    std::unique_ptr<ImageReader>* reader) {
  auto result = impl_->AdjustReaderAfterInfoReady(reader);
  if (!result.ok() || !next_)
    return result;
  return next_->AdjustReaderAfterInfoReady(reader);
}

Result LayeredAdjuster::AdjustWriter(ImageReader* reader,
                                     std::unique_ptr<ImageWriter>* writer) {
  auto result = impl_->AdjustWriter(reader, writer);
  if (!result.ok() || !next_)
    return result;
  return next_->AdjustWriter(reader, writer);
}

bool LayeredAdjuster::ShouldWaitForMetadata() {
  auto should_wait = impl_->ShouldWaitForMetadata();
  if (should_wait || !next_)
    return should_wait;
  return next_->ShouldWaitForMetadata();
}

void LayeredAdjuster::AdjustGifDecoderParams(GifDecoder::Params* params) {
  impl_->AdjustGifDecoderParams(params);
  if (next_)
    next_->AdjustGifDecoderParams(params);
}

void LayeredAdjuster::AdjustJpegDecoderParams(JpegDecoder::Params* params) {
  impl_->AdjustJpegDecoderParams(params);
  if (next_)
    next_->AdjustJpegDecoderParams(params);
}

void LayeredAdjuster::AdjustPngDecoderParams(PngDecoder::Params* params) {
  impl_->AdjustPngDecoderParams(params);
  if (next_)
    next_->AdjustPngDecoderParams(params);
}

void LayeredAdjuster::AdjustWebPDecoderParams(WebPDecoder::Params* params) {
  impl_->AdjustWebPDecoderParams(params);
  if (next_)
    next_->AdjustWebPDecoderParams(params);
}

void LayeredAdjuster::AdjustWebPEncoderParams(WebPEncoder::Params* params) {
  impl_->AdjustWebPEncoderParams(params);
  if (next_)
    next_->AdjustWebPEncoderParams(params);
}

NullAdjuster::NullAdjuster() {}

NullAdjuster::~NullAdjuster() {}

Result NullAdjuster::ShouldEvenBother() {
  return Result::Ok();
}

Result NullAdjuster::AdjustReader(ImageType image_type,
                                  std::unique_ptr<ImageReader>* reader) {
  return Result::Ok();
}

Result NullAdjuster::AdjustReaderAfterInfoReady(
    std::unique_ptr<ImageReader>* reader) {
  return Result::Ok();
}

Result NullAdjuster::AdjustWriter(ImageReader* reader,
                                  std::unique_ptr<ImageWriter>* writer) {
  return Result::Ok();
}

bool NullAdjuster::ShouldWaitForMetadata() {
  return false;
}

void NullAdjuster::AdjustGifDecoderParams(GifDecoder::Params* params) {}

void NullAdjuster::AdjustJpegDecoderParams(JpegDecoder::Params* params) {}

void NullAdjuster::AdjustPngDecoderParams(PngDecoder::Params* params) {}

void NullAdjuster::AdjustWebPDecoderParams(WebPDecoder::Params* params) {}

void NullAdjuster::AdjustWebPEncoderParams(WebPEncoder::Params* params) {}

}  // namespace image
