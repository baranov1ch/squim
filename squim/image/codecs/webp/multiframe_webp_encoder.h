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

#ifndef SQUIM_IMAGE_CODECS_WEBP_MULTIFRAME_WEBP_ENCODER_H_
#define SQUIM_IMAGE_CODECS_WEBP_MULTIFRAME_WEBP_ENCODER_H_

#include "google/libwebp/upstream/src/webp/encode.h"
#include "google/libwebp/upstream/examples/gif2webp_util.h"
#include "squim/image/codecs/webp_encoder.h"
#include "squim/image/image_metadata.h"

namespace image {

class MultiframeWebPEncoder : public WebPEncoder::Impl {
 public:
  MultiframeWebPEncoder(WebPEncoder::Params* params, io::VectorWriter* output);
  ~MultiframeWebPEncoder() override;

  // WebPEncoder::Impl implementation:
  void SetImageInfo(const ImageInfo* image_info) override;
  void SetMetadata(const ImageMetadata* metadata) override;
  Result EncodeFrame(ImageFrame* frame) override;
  Result FinishEncoding() override;
  void GetStats(ImageOptimizationStats* stats) override;

 private:
  Result InitMuxer();
  void SetMetadataIfNeeded(bool needed,
                           const char fourcc[4],
                           ImageMetadata::Type type);

  WebPEncoder::Params* params_;
  io::VectorWriter* output_;
  const ImageInfo* image_info_ = nullptr;
  const ImageMetadata* metadata_ = nullptr;
  WebPConfig webp_config_;
  size_t next_frame_idx_ = 0;
  WebPPicture webp_image_;
  WebPPicture webp_frame_;
  WebPFrameCache* webp_frame_cache_ = nullptr;
  WebPMux* webp_mux_ = nullptr;
  std::unique_ptr<WebPAuxStats> stats_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_CODECS_WEBP_MULTIFRAME_WEBP_ENCODER_H_
