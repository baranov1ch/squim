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

#ifndef SQUIM_IMAGE_CODECS_GIF_DECODER_H_
#define SQUIM_IMAGE_CODECS_GIF_DECODER_H_

#include <memory>
#include <vector>

#include "squim/base/make_noncopyable.h"
#include "squim/image/codecs/decode_params.h"
#include "squim/image/image_decoder.h"
#include "squim/image/image_frame.h"
#include "squim/image/image_info.h"
#include "squim/image/image_metadata.h"

namespace io {
class BufReader;
}

namespace image {

class GifDecoder : public ImageDecoder {
  MAKE_NONCOPYABLE(GifDecoder);

 public:
  struct Params : public DecodeParams {
    static Params Default();
  };

  GifDecoder(Params params, std::unique_ptr<io::BufReader> source);
  ~GifDecoder() override;

  // ImageDecoder implementation:
  bool IsImageInfoComplete() const override;
  const ImageInfo& GetImageInfo() const override;
  bool IsFrameHeaderCompleteAtIndex(size_t index) const override;
  bool IsFrameCompleteAtIndex(size_t index) const override;
  ImageFrame* GetFrameAtIndex(size_t index) override;
  size_t GetFrameCount() const override;
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

  ImageInfo image_info_;
  std::vector<std::unique_ptr<ImageFrame>> image_frames_;
  ImageMetadata metadata_;
  std::unique_ptr<io::BufReader> source_;
  std::unique_ptr<Impl> impl_;
  Result decode_error_ = Result::Ok();
  Params params_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_CODECS_GIF_DECODER_H_
