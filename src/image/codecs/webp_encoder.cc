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

#include "image/codecs/webp_encoder.h"

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "google/libwebp/upstream/src/webp/encode.h"
#include "google/libwebp/upstream/examples/gif2webp_util.h"
#include "image/image_frame.h"
#include "image/image_info.h"
#include "image/pixel.h"
#include "io/writer.h"

namespace image {

namespace {

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

uint32_t PackAsARGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (static_cast<uint32_t>(a) << 24) | (r << 16) | (g << 8) | (b << 0);
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

// Reader for 4:2:0 YUV-encoded image with alpha (optionally).
class YUVAReader {
 public:
  YUVAReader(ImageFrame* frame) {
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

class Picture {
 public:
  Picture() {}

  ~Picture() {
    if (owns_data_)
      WebPPictureFree(&picture_);
  }

  bool Init(ImageFrame* frame) {
    if (!WebPPictureInit(&picture_))
      return false;

    std::unique_ptr<ImageFrame> transformed_frame;
    if (frame->is_grayscale()) {
      // TODO: transform to RGB(A).
      transformed_frame.reset(new ImageFrame);
      ConvertGrayToRGB(frame, transformed_frame.get());
      frame = transformed_frame.get();
    }

    if (!frame->is_yuv() && !frame->is_rgb())
      return false;

    picture_.width = frame->width();
    picture_.height = frame->height();

    bool result = false;
    switch (frame->color_scheme()) {
      case ColorScheme::kRGB:
        result =
            WebPPictureImportRGB(&picture_, frame->GetData(0), frame->stride());
        owns_data_ = true;
        break;
      case ColorScheme::kRGBA:
        result = WebPPictureImportRGBA(&picture_, frame->GetData(0),
                                       frame->stride());
        owns_data_ = true;
        break;
      case ColorScheme::kYUV:
      case ColorScheme::kYUVA:
        result = WebPPictureFromYUVAFrame(frame, &picture_);
        break;
      default:
        NOTREACHED();
    }

    if (!result)
      return false;

    return true;
  }

  WebPPicture* webp_picture() { return &picture_; }

 private:
  bool owns_data_ = false;
  WebPPicture picture_;
};

}  // namespace

// static
WebPEncoder::Params WebPEncoder::Params::Default() {
  return Params();
}

class WebPEncoder::Impl {
 public:
  Impl(WebPEncoder* encoder) : encoder_(encoder) {}
  ~Impl() {
    if (mode_ == Mode::kMultiFrame) {
      WebPFrameCacheDelete(webp_frame_cache_);
      WebPMuxDelete(webp_mux_);
      WebPPictureFree(&webp_image_);
    }
  }

  Result EncodeSingle(ImageFrame* frame) {
    CHECK_EQ(Mode::kNotDecidedYet, mode_);
    mode_ = Mode::kSingleFrame;

    WebPConfig config;
    const auto& params = encoder_->params_;

    auto preset = PresetToWebPPreset(params.preset);
    if (!WebPConfigPreset(&config, preset, params.quality))
      return Result::Error(Result::Code::kEncodeError);

    config.method = params.method;

    if (!WebPValidateConfig(&config))
      return Result::Error(Result::Code::kEncodeError);

    Picture picture;
    if (!picture.Init(frame))
      return Result::Error(Result::Code::kEncodeError);

    picture.webp_picture()->writer = ChunkWriter;
    picture.webp_picture()->custom_ptr = this;
    picture.webp_picture()->progress_hook = ProgressHook;
    picture.webp_picture()->user_data = this;

    // Now take picture and WebP encode it.
    bool result = WebPEncode(&config, picture.webp_picture());

    return result ? Result::Ok() : Result::Error(Result::Code::kEncodeError);
  }

  Result EncodeNextFrame(ImageFrame* frame) {
    if (mode_ == Mode::kNotDecidedYet) {
      auto result = InitMuxer();
      if (!result.ok())
        return result;
    }

    CHECK_EQ(Mode::kMultiFrame, mode_);

    if (!WebPPictureView(&webp_image_, frame->x_offset(), frame->y_offset(),
                         frame->width(), frame->height(), &webp_frame_))
      return Result::Error(Result::Code::kEncodeError);

    auto* where = webp_frame_.argb;
    Bitmap bitmap(frame);
    switch (frame->color_scheme()) {
      case ColorScheme::kGrayScale:
        for (uint32_t y = 0; y < frame->height(); ++y) {
          for (uint32_t x = 0; x < frame->width(); ++x) {
            auto px = bitmap.GetPixel<GrayScalePixel>(x, y);
            *where++ = PackAsARGB(px.g(), px.g(), px.g(), 0xFF);
          }
        }
        break;
      case ColorScheme::kGrayScaleAlpha:
        for (uint32_t y = 0; y < frame->height(); ++y) {
          for (uint32_t x = 0; x < frame->width(); ++x) {
            auto px = bitmap.GetPixel<GrayScaleAlphaPixel>(x, y);
            *where++ = PackAsARGB(px.g(), px.g(), px.g(), px.a());
          }
        }
        break;
      case ColorScheme::kRGB:
        for (uint32_t y = 0; y < frame->height(); ++y) {
          for (uint32_t x = 0; x < frame->width(); ++x) {
            auto px = bitmap.GetPixel<RGBPixel>(x, y);
            *where++ = PackAsARGB(px.r(), px.g(), px.b(), 0xFF);
          }
        }
        break;
      case ColorScheme::kRGBA:
        for (uint32_t y = 0; y < frame->height(); ++y) {
          for (uint32_t x = 0; x < frame->width(); ++x) {
            auto px = bitmap.GetPixel<RGBAPixel>(x, y);
            *where++ = PackAsARGB(px.r(), px.g(), px.b(), px.a());
          }
        }
        break;
      default:
        return Result::Error(Result::Code::kEncodeError);
    }

    // We need to pass image to add frame.
    WebPFrameRect frame_rect = {static_cast<int>(frame->x_offset()),
                                static_cast<int>(frame->y_offset()),
                                static_cast<int>(frame->width()),
                                static_cast<int>(frame->height())};
    auto disposal_method =
        DisposalMethodToWebPDisposal(frame->disposal_method());

    if (!WebPFrameCacheAddFrame(webp_frame_cache_, &webp_config_, &frame_rect,
                                disposal_method, frame->duration(),
                                &webp_image_)) {
      return Result::Error(Result::Code::kEncodeError);
    }

    if (WebPFrameCacheFlush(webp_frame_cache_, false /*verbose*/, webp_mux_) !=
        WEBP_MUX_OK)
      return Result::Error(Result::Code::kEncodeError);

    next_frame_idx_++;

    return Result::Ok();
  }

  Result FinishEncoding() {
    if (mode_ != Mode::kMultiFrame)
      return Result::Ok();

    if (WebPFrameCacheFlushAll(webp_frame_cache_, false /*verbose*/,
                               webp_mux_) != WEBP_MUX_OK)
      return Result::Error(Result::Code::kEncodeError);

    if (next_frame_idx_ > 1) {
      RGBAPixel bg(const_cast<uint8_t*>(image_info_->bg_color->data()));
      // This was an animated image.
      WebPMuxAnimParams anim = {PackAsARGB(bg.r(), bg.g(), bg.b(), bg.a()),
                                static_cast<int>(image_info_->loop_count)};
      if (WebPMuxSetAnimationParams(webp_mux_, &anim) != WEBP_MUX_OK)
        return Result::Error(Result::Code::kEncodeError);
    }

    WebPData webp_data = {NULL, 0};
    if (WebPMuxAssemble(webp_mux_, &webp_data) != WEBP_MUX_OK)
      return Result::Error(Result::Code::kEncodeError);

    auto chunk = io::Chunk::Copy(webp_data.bytes, webp_data.size);
    encoder_->output_.push_back(std::move(chunk));
    WebPDataClear(&webp_data);

    return Result::Ok();
  }

  bool idle() const { return mode_ == Mode::kNotDecidedYet; }
  void set_image_info(const ImageInfo* image_info) { image_info_ = image_info; }

 private:
  enum Mode { kNotDecidedYet, kSingleFrame, kMultiFrame };

  static int ProgressHook(int percent, const WebPPicture* picture) {
    // TODO: handle timeouts.
    return 1;
  }

  static int ChunkWriter(const uint8_t* data,
                         size_t data_size,
                         const WebPPicture* const picture) {
    auto* impl = static_cast<Impl*>(picture->custom_ptr);
    auto chunk = io::Chunk::Copy(data, data_size);
    impl->encoder_->output_.push_back(std::move(chunk));
    return 1;
  }

  Result InitMuxer() {
    CHECK_EQ(Mode::kNotDecidedYet, mode_);
    CHECK(image_info_);

    webp_mux_ = WebPMuxNew();
    if (!webp_mux_)
      return Result::Error(Result::Code::kEncodeError);

    const auto& params = encoder_->params_;

    auto preset = PresetToWebPPreset(params.preset);
    if (!WebPConfigPreset(&webp_config_, preset, params.quality))
      return Result::Error(Result::Code::kEncodeError);

    webp_config_.method = params.method;

    if (!WebPValidateConfig(&webp_config_))
      return Result::Error(Result::Code::kEncodeError);

    if (!WebPPictureInit(&webp_image_))
      return Result::Error(Result::Code::kEncodeError);

    webp_image_.width = image_info_->width;
    webp_image_.height = image_info_->height;
    webp_image_.use_argb = true;

    if (!WebPPictureAlloc(&webp_image_))
      return Result::Error(Result::Code::kEncodeError);

    WebPUtilClearPic(&webp_image_, nullptr);

    webp_image_.progress_hook = ProgressHook;
    webp_image_.user_data = this;

    // Key frame parameters: do not insert unnecessary key frames.
    static const size_t kMax = ~0;
    static const size_t kMin = kMax - 1;
    webp_frame_cache_ =
        WebPFrameCacheNew(image_info_->width, image_info_->height, kMin, kMax,
                          params.compression == Compression::kMixed);

    mode_ = Mode::kMultiFrame;

    return Result::Ok();
  }

  WebPEncoder* encoder_;
  Mode mode_ = Mode::kNotDecidedYet;
  const ImageInfo* image_info_ = nullptr;
  size_t next_frame_idx_ = 0;
  WebPPicture webp_image_;
  WebPPicture webp_frame_;
  WebPFrameCache* webp_frame_cache_ = nullptr;
  WebPMux* webp_mux_ = nullptr;
  WebPConfig webp_config_;
};

WebPEncoder::WebPEncoder(Params params, std::unique_ptr<io::VectorWriter> dst)
    : params_(params), dst_(std::move(dst)) {
  impl_ = base::make_unique<Impl>(this);
}

WebPEncoder::~WebPEncoder() {}

Result WebPEncoder::Initialize(const ImageInfo* image_info) {
  impl_->set_image_info(image_info);
  return Result::Ok();
}

Result WebPEncoder::EncodeFrame(ImageFrame* frame, bool last_frame) {
  if (!error_.ok())
    return error_;

  if (impl_->idle() && last_frame && frame) {
    // Single frame image - use simple API.
    auto result = impl_->EncodeSingle(frame);
    if (result.error()) {
      error_ = result;
      return result;
    }
  } else {
    // Otherwise, mess with WebP Muxer API.
    if (frame) {
      auto result = impl_->EncodeNextFrame(frame);
      if (result.error()) {
        error_ = result;
        return result;
      }
    }

    if (last_frame) {
      auto result = impl_->FinishEncoding();
      if (result.error()) {
        error_ = result;
        return result;
      }
    }
  }

  if (!output_.empty()) {
    auto io_result = dst_->WriteV(std::move(output_));
    output_ = io::ChunkList();
    auto result = Result::FromIoResult(io_result, false);
    if (result.error()) {
      error_ = result;
      return result;
    }
  }

  return Result::Ok();
}

void WebPEncoder::SetMetadata(const ImageMetadata* metadata) {}

Result WebPEncoder::FinishWrite(ImageWriter::Stats* stats) {
  return Result::Ok();
}

}  // namespace image
