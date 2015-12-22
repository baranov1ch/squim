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

#ifndef IMAGE_CODECS_JPEG_DECODER_H_
#define IMAGE_CODECS_JPEG_DECODER_H_

#include <memory>

#include "base/make_noncopyable.h"
#include "image/codecs/decode_params.h"
#include "image/image_decoder.h"
#include "image/image_frame.h"
#include "image/image_metadata.h"

namespace io {
class BufReader;
}

namespace image {

class JpegDecoder : public ImageDecoder {
  MAKE_NONCOPYABLE(JpegDecoder);

 public:
  struct Params : public DecodeParams {
    static Params Default();
  };

  JpegDecoder(Params params, std::unique_ptr<io::BufReader> source);
  ~JpegDecoder() override;

  // ImageDecoder implementation:
  uint32_t GetWidth() const override;
  uint32_t GetHeight() const override;
  uint64_t GetSize() const override;
  ImageType GetImageType() const override;
  ColorScheme GetColorScheme() const override;
  bool IsProgressive() const override;
  bool IsImageInfoComplete() const override;
  size_t GetFrameCount() const override;
  bool IsMultiFrame() const override;
  uint32_t GetEstimatedQuality() const override;
  bool IsFrameCompleteAtIndex(size_t index) const override;
  ImageFrame* GetFrameAtIndex(size_t index) override;
  ImageMetadata* GetMetadata() override;
  bool IsAllMetadataComplete() const override;
  bool IsAllFramesComplete() const override;
  bool IsImageComplete() const override;
  Result Decode() override;
  Result DecodeImageInfo() override;
  bool HasError() const override;

 private:
  class Impl;

  void Fail(Result error);
  Result ProcessDecodeResult(bool result);

  io::BufReader* source() { return source_.get(); }

  ImageFrame* frame() { return &image_frame_; }

  void set_size(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
  }

  void set_is_progressive(bool value) { is_progressive_ = value; }

  void set_color_space(ColorScheme color_scheme) {
    color_scheme_ = color_scheme;
  }

  ImageFrame image_frame_;
  ImageMetadata metadata_;
  std::unique_ptr<io::BufReader> source_;
  std::unique_ptr<Impl> impl_;
  ColorScheme color_scheme_ = ColorScheme::kUnknown;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  bool is_progressive_ = false;
  Result decode_error_ = Result::Ok();
  Params params_;
};

}  // namespace image

#endif  // IMAGE_CODECS_JPEG_DECODER_H_
