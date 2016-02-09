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

#include "squim/image/optimization/convert_to_webp_strategy.h"

#include "squim/base/logging.h"
#include "squim/image/decoding_reader.h"
#include "squim/image/image_codec_factory.h"
#include "squim/image/optimization/lazy_webp_writer.h"
#include "squim/io/buf_reader.h"
#include "squim/io/writer.h"

namespace image {

namespace {

template <typename Params>
void AddSupportedColorSchemes(Params* params) {
  DCHECK(params);
  params->allowed_color_schemes.insert(ColorScheme::kRGB);
  params->allowed_color_schemes.insert(ColorScheme::kRGBA);
}
}

ConvertToWebPStrategy::ConvertToWebPStrategy() {}

ConvertToWebPStrategy::~ConvertToWebPStrategy() {}

Result ConvertToWebPStrategy::ShouldEvenBother() {
  return Result::Ok();
}

Result ConvertToWebPStrategy::CreateImageReader(
    ImageType image_type,
    std::unique_ptr<io::BufReader> src,
    std::unique_ptr<ImageReader>* reader) {
  auto decoder = codec_factory_->CreateDecoder(image_type, std::move(src));
  if (!decoder)
    return Result::Error(Result::Code::kUnsupportedFormat);

  reader->reset(new DecodingReader(std::move(decoder)));
  return Result::Ok();
}

Result ConvertToWebPStrategy::CreateImageWriter(
    std::unique_ptr<io::VectorWriter> dest,
    ImageReader* reader,
    std::unique_ptr<ImageWriter>* writer) {
  if (!codec_factory_)
    return Result::Error(Result::Code::kFailed, "Not yet configured");
  const ImageInfo* image_info;
  auto result = reader->GetImageInfo(&image_info);
  DCHECK(result.ok());
  if (image_info->type == ImageType::kWebP)
    return Result::Error(Result::Code::kDunnoHowToEncode,
                         "WebP is not supported yet");

  if (image_info->type == ImageType::kGif)
    allow_mixed_ = true;

  writer->reset(
      new LazyWebPWriter(std::move(dest), codec_factory_, image_info));
  return Result::Ok();
}

Result ConvertToWebPStrategy::AdjustImageReaderAfterInfoReady(
    std::unique_ptr<ImageReader>* reader) {
  return Result::Ok();
}

bool ConvertToWebPStrategy::ShouldWaitForMetadata() {
  return false;
}

GifDecoder::Params ConvertToWebPStrategy::GetGifDecoderParams() {
  GifDecoder::Params params;
  AddSupportedColorSchemes(&params);
  return params;
}

JpegDecoder::Params ConvertToWebPStrategy::GetJpegDecoderParams() {
  JpegDecoder::Params params;
  AddSupportedColorSchemes(&params);
  return params;
}

PngDecoder::Params ConvertToWebPStrategy::GetPngDecoderParams() {
  PngDecoder::Params params;
  AddSupportedColorSchemes(&params);
  return params;
}

WebPDecoder::Params ConvertToWebPStrategy::GetWebPDecoderParams() {
  return WebPDecoder::Params::Default();
}

WebPEncoder::Params ConvertToWebPStrategy::GetWebPEncoderParams() {
  auto params = WebPEncoder::Params::Default();
  if (allow_mixed_)
    params.compression = WebPEncoder::Compression::kMixed;
  return params;
}

void ConvertToWebPStrategy::SetCodecFactory(ImageCodecFactory* factory) {
  codec_factory_ = factory;
}

}  // namespace image
