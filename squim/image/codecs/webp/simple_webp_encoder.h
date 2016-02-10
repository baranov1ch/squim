/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#ifndef SQUIM_IMAGE_CODECS_WEBP_SIMPLE_WEBP_ENCODER_H_
#define SQUIM_IMAGE_CODECS_WEBP_SIMPLE_WEBP_ENCODER_H_

#include "google/libwebp/upstream/src/webp/encode.h"
#include "squim/image/codecs/webp_encoder.h"

namespace image {

class SimpleWebPEncoder : public WebPEncoder::Impl {
 public:
  SimpleWebPEncoder(WebPEncoder::Params* params, io::VectorWriter* output);
  ~SimpleWebPEncoder() override;

  // WebPEncoder::Impl implementation:
  void SetImageInfo(const ImageInfo* image_info) override;
  void SetMetadata(const ImageMetadata* metadata) override;
  Result EncodeFrame(ImageFrame* frame) override;
  Result FinishEncoding() override;
  void GetStats(ImageOptimizationStats* stats) override;

 private:
  static int ChunkWriter(const uint8_t* data,
                         size_t data_size,
                         const WebPPicture* const picture);

  WebPEncoder::Params* params_;
  io::VectorWriter* output_;
  const ImageInfo* image_info_ = nullptr;
  const ImageMetadata* metadata_ = nullptr;
  WebPConfig webp_config_;
  WebPPicture picture_;
  bool owns_data_ = false;
  std::unique_ptr<WebPAuxStats> stats_;
  io::ChunkList chunks_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_CODECS_WEBP_SIMPLE_WEBP_ENCODER_H_
