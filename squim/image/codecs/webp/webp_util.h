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

#ifndef SQUIM_IMAGE_CODECS_WEBP_WEBP_UTIL_H_
#define SQUIM_IMAGE_CODECS_WEBP_WEBP_UTIL_H_

#include "google/libwebp/upstream/src/webp/encode.h"
#include "google/libwebp/upstream/examples/gif2webp_util.h"
#include "squim/image/codecs/webp_encoder.h"
#include "squim/image/image_frame.h"

namespace image {

std::string WebPError(const std::string& prefix, WebPPicture* picture);

// Reader for 4:2:0 YUV-encoded image with alpha (optionally).
class YUVAReader {
 public:
  YUVAReader(ImageFrame* frame);

  uint8_t* y() { return y_; }
  uint8_t* u() { return u_; }
  uint8_t* v() { return v_; }
  uint8_t* a() { return a_; }

  uint32_t y_stride() const { return y_stride_; }
  uint32_t uv_stride() const { return uv_stride_; }
  uint32_t a_stride() const { return a_stride_; }

 private:
  uint8_t* y_;
  uint32_t y_stride_;
  uint8_t* u_;
  uint8_t* v_;
  uint32_t uv_stride_;
  uint8_t* a_;
  uint32_t a_stride_;
};

WebPImageHint HintToWebPImageHint(WebPEncoder::Hint hint);

WebPPreset PresetToWebPPreset(WebPEncoder::Preset preset);

FrameDisposeMethod DisposalMethodToWebPDisposal(
    ImageFrame::DisposalMethod disposal);

inline uint32_t PackAsARGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (static_cast<uint32_t>(a) << 24) | (r << 16) | (g << 8) | (b << 0);
}

void ConvertGrayToRGBA(ImageFrame* in, ImageFrame* out);

void ConvertGrayToRGB(ImageFrame* in, ImageFrame* out);

bool WebPPictureFromYUVAFrame(ImageFrame* frame, WebPPicture* picture);

Result EncoderParamsToWebPConfig(const WebPEncoder::Params& params,
                                 WebPConfig* webp_config,
                                 double image_quality);

int WebPProgressHook(int percent, const WebPPicture* picture);

}  // namespace image

#endif  // SQUIM_IMAGE_CODECS_WEBP_WEBP_UTIL_H_
