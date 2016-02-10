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

#include "squim/image/codecs/webp/webp_util.h"

#include "squim/image/pixel.h"

namespace image {

namespace {

const char* const kErrorMessages[VP8_ENC_ERROR_LAST] = {
    "OK", "OUT_OF_MEMORY: Out of memory allocating objects",
    "BITSTREAM_OUT_OF_MEMORY: Out of memory re-allocating byte buffer",
    "NULL_PARAMETER: NULL parameter passed to function",
    "INVALID_CONFIGURATION: configuration is invalid",
    "BAD_DIMENSION: Bad picture dimension. Maximum width and height "
    "allowed is 16383 pixels.",
    "PARTITION0_OVERFLOW: Partition #0 is too big to fit 512k.\n"
    "To reduce the size of this partition, try using less segments "
    "with the -segments option, and eventually reduce the number of "
    "header bits using -partition_limit. More details are available "
    "in the manual (`man cwebp`)",
    "PARTITION_OVERFLOW: Partition is too big to fit 16M",
    "BAD_WRITE: Picture writer returned an I/O error",
    "FILE_TOO_BIG: File would be too big to fit in 4G",
    "USER_ABORT: Timeout occured",
};

}  // namespace

std::string WebPError(const std::string& prefix, WebPPicture* picture) {
  return prefix + kErrorMessages[picture->error_code];
}

YUVAReader::YUVAReader(ImageFrame* frame) {
  uint8_t* mem = frame->GetData(0);
  y_stride_ = frame->width();
  y_ = mem;
  mem += y_stride_;

  uv_stride_ = (frame->width() + 1) >> 1;
  u_ = mem;
  mem += uv_stride_;
  v_ = mem;
  mem += uv_stride_;

  if (frame->has_alpha()) {
    a_ = mem;
    a_stride_ = frame->width();
  } else {
    a_ = nullptr;
    a_stride_ = 0;
  }
}

WebPImageHint HintToWebPImageHint(WebPEncoder::Hint hint) {
  switch (hint) {
    case WebPEncoder::Hint::kDefault:
      return WEBP_HINT_DEFAULT;
    case WebPEncoder::Hint::kPicture:
      return WEBP_HINT_PICTURE;
    case WebPEncoder::Hint::kPhoto:
      return WEBP_HINT_PHOTO;
    case WebPEncoder::Hint::kGraph:
      return WEBP_HINT_GRAPH;
    default:
      NOTREACHED() << "Unknown WebP image hint: " << static_cast<int>(hint);
      return WEBP_HINT_DEFAULT;
  }
}

WebPPreset PresetToWebPPreset(WebPEncoder::Preset preset) {
  switch (preset) {
    case WebPEncoder::Preset::kDefault:
      return WEBP_PRESET_DEFAULT;
    case WebPEncoder::Preset::kPicture:
      return WEBP_PRESET_PICTURE;
    case WebPEncoder::Preset::kPhoto:
      return WEBP_PRESET_PHOTO;
    case WebPEncoder::Preset::kDrawing:
      return WEBP_PRESET_DRAWING;
    case WebPEncoder::Preset::kIcon:
      return WEBP_PRESET_ICON;
    case WebPEncoder::Preset::kText:
      return WEBP_PRESET_TEXT;
    default:
      NOTREACHED() << "Unknown WebP preset: " << static_cast<int>(preset);
      return WEBP_PRESET_DEFAULT;
  }
}

FrameDisposeMethod DisposalMethodToWebPDisposal(
    ImageFrame::DisposalMethod disposal) {
  switch (disposal) {
    case ImageFrame::DisposalMethod::kBackground:
      return FRAME_DISPOSE_BACKGROUND;
    case ImageFrame::DisposalMethod::kRestorePrevious:
      return FRAME_DISPOSE_RESTORE_PREVIOUS;
    case ImageFrame::DisposalMethod::kNone:
      return FRAME_DISPOSE_NONE;
    default:
      NOTREACHED();
      return FRAME_DISPOSE_NONE;
  }
}

void ConvertGrayToRGBA(ImageFrame* in, ImageFrame* out) {
  Bitmap from(in);
  Bitmap to(out);
  auto width = in->width();
  auto height = in->height();
  out->set_size(width, height);
  out->set_offset(in->x_offset(), in->y_offset());
  out->set_color_scheme(ColorScheme::kRGBA);
  out->Init();
  if (in->has_alpha()) {
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        const auto gray = from.GetPixel<GrayScaleAlphaPixel>(x, y);
        auto rgba = to.GetPixel<RGBAPixel>(x, y);
        rgba.set(gray.g(), gray.g(), gray.g(), gray.a());
      }
    }
  } else {
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        const auto gray = from.GetPixel<GrayScalePixel>(x, y);
        auto rgba = to.GetPixel<RGBAPixel>(x, y);
        rgba.set(gray.g(), gray.g(), gray.g(), 0xFF);
      }
    }
  }
}

void ConvertGrayToRGB(ImageFrame* in, ImageFrame* out) {
  Bitmap from(in);
  Bitmap to(out);
  auto width = in->width();
  auto height = in->height();
  out->set_size(width, height);
  out->set_offset(in->x_offset(), in->y_offset());

  if (in->has_alpha()) {
    out->set_color_scheme(ColorScheme::kRGBA);
    out->Init();
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        const auto gray = from.GetPixel<GrayScaleAlphaPixel>(x, y);
        auto rgba = to.GetPixel<RGBAPixel>(x, y);
        rgba.set(gray.g(), gray.g(), gray.g(), gray.a());
      }
    }
  } else {
    out->set_color_scheme(ColorScheme::kRGB);
    out->Init();
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        const auto gray = from.GetPixel<GrayScalePixel>(x, y);
        auto rgb = to.GetPixel<RGBPixel>(x, y);
        rgb.set(gray.g(), gray.g(), gray.g());
      }
    }
  }
}

bool WebPPictureFromYUVAFrame(ImageFrame* frame, WebPPicture* picture) {
  if (!frame->is_yuv())
    return false;

  picture->width = frame->width();
  picture->height = frame->height();

  YUVAReader frame_reader(frame);
  picture->y = frame_reader.y();
  picture->y_stride = frame_reader.y_stride();
  picture->u = frame_reader.u();
  picture->v = frame_reader.v();
  picture->uv_stride = frame_reader.uv_stride();
  picture->a = frame_reader.a();
  picture->a_stride = frame_reader.a_stride();

  picture->colorspace = frame->has_alpha() ? WEBP_YUV420A : WEBP_YUV420;
  return true;
}

Result EncoderParamsToWebPConfig(const WebPEncoder::Params& params,
                                 WebPConfig* webp_config,
                                 double image_quality) {
  auto quality = params.quality;
  if (image_quality != ImageFrame::kUnknownQuality && quality > image_quality) {
    quality = image_quality;
  }

  auto preset = PresetToWebPPreset(params.preset);
  if (!WebPConfigPreset(webp_config, preset, quality))
    return Result::Error(Result::Code::kEncodeError,
                         "WebP config preset error");

  webp_config->method = params.method;

  if (!WebPValidateConfig(webp_config))
    return Result::Error(Result::Code::kEncodeError,
                         "WebP config validation error");

  return Result::Ok();
}

int WebPProgressHook(int percent, const WebPPicture* picture) {
  auto* params = static_cast<WebPEncoder::Params*>(picture->user_data);
  if (params->progress_cb) {
    return params->progress_cb() ? 1 : 0;
  }
  return 1;
}

}  // namespace image
