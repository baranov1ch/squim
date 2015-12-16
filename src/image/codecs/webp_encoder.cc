#include "image/codecs/webp_encoder.h"

#include "base/memory/make_unique.h"
#include "glog/logging.h"
#include "google/libwebp/upstream/src/webp/encode.h"
#include "image/image_frame.h"
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
      DCHECK(false) << "Unknown WebP image hint: " << static_cast<int>(hint);
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
      DCHECK(false) << "Unknown WebP preset: " << static_cast<int>(preset);
      return WEBP_PRESET_DEFAULT;
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
        DCHECK(false);
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

class WebPEncoder::Impl {
 public:
  Impl(WebPEncoder* encoder) : encoder_(encoder) {}
  ~Impl() {}

  Result EncodeSingle(ImageFrame* frame) {
    CHECK(idle_);
    idle_ = false;

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

    // Now we need to take picture and WebP encode it.
    bool result = WebPEncode(&config, picture.webp_picture());

    return result ? Result::Ok() : Result::Error(Result::Code::kEncodeError);
  }

  Result EncodeNextFrame(ImageFrame* frame) {
    idle_ = false;
    return Result::Ok();
  }

  Result FinishEncoding() {
    idle_ = false;
    return Result::Ok();
  }

  bool idle() const { return idle_; }

 private:
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

  bool idle_ = true;
  WebPEncoder* encoder_;
};

WebPEncoder::WebPEncoder(Params params, std::unique_ptr<io::VectorWriter> dst)
    : params_(params), dst_(std::move(dst)) {
  impl_ = base::make_unique<Impl>(this);
}

WebPEncoder::~WebPEncoder() {}

Result WebPEncoder::EncodeFrame(ImageFrame* frame, bool last_frame) {
  if (impl_->idle() && last_frame && frame) {
    // Single frame image - use simple API.
    auto result = impl_->EncodeSingle(frame);
    if (!result.ok())
      return result;
  } else {
    // Otherwise, mess with WebP Muxer API.
    if (frame) {
      auto result = impl_->EncodeNextFrame(frame);
      if (!result.ok())
        return result;
    }

    if (last_frame) {
      auto result = impl_->FinishEncoding();
      if (!result.ok())
        return result;
    }
  }

  if (!output_.empty()) {
    auto io_result = dst_->WriteV(std::move(output_));
    output_ = io::ChunkList();
    return Result::FromIoResult(io_result, false);
  }

  return Result::Ok();
}

void WebPEncoder::SetMetadata(const ImageMetadata* metadata) {}

Result WebPEncoder::FinishWrite() {
  return Result::Ok();
}

}  // namespace image
