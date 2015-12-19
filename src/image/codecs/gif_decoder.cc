#include "image/codecs/gif_decoder.h"

#include "base/memory/make_unique.h"
#include "glog/logging.h"
#include "io/buf_reader.h"

namespace image {

// static
GifDecoder::Params GifDecoder::Params::Default() {
  Params params;
  params.allowed_color_schemes.insert(ColorScheme::kRGB);
  params.allowed_color_schemes.insert(ColorScheme::kRGBA);
  params.allowed_color_schemes.insert(ColorScheme::kGrayScale);
  params.allowed_color_schemes.insert(ColorScheme::kGrayScaleAlpha);
  return params;
}

class GifDecoder::Impl {
  MAKE_NONCOPYABLE(Impl);

 public:
  Impl(GifDecoder* decoder) : decoder_(decoder) {}
  ~Impl() {}

  bool Decode(bool header_only) { return true; }

  bool HeaderComplete() const { return true; }

  bool ImageComplete() const { return true; }

  bool DecodingComplete() const { return true; }

 private:
  GifDecoder* decoder_;
};

GifDecoder::GifDecoder(Params params, std::unique_ptr<io::BufReader> source)
    : source_(std::move(source)), params_(params) {
  impl_ = base::make_unique<Impl>(this);
}

GifDecoder::~GifDecoder() {}

uint32_t GifDecoder::GetWidth() const {
  return width_;
}

uint32_t GifDecoder::GetHeight() const {
  return height_;
}

uint64_t GifDecoder::GetSize() const {
  // TODO:
  return 0;
}

ImageType GifDecoder::GetImageType() const {
  return ImageType::kGif;
}

ColorScheme GifDecoder::GetColorScheme() const {
  return color_scheme_;
}

bool GifDecoder::IsProgressive() const {
  return is_progressive_;
}

bool GifDecoder::IsImageInfoComplete() const {
  return impl_->HeaderComplete();
}

size_t GifDecoder::GetFrameCount() const {
  return image_frames_.size();
}

bool GifDecoder::IsMultiFrame() const {
  return true;
}

uint32_t GifDecoder::GetEstimatedQuality() const {
  return 100;
}

bool GifDecoder::IsFrameCompleteAtIndex(size_t index) const {
  if (index >= image_frames_.size())
    return false;

  return image_frames_[index]->status() == ImageFrame::Status::kComplete;
}

ImageFrame* GifDecoder::GetFrameAtIndex(size_t index) {
  CHECK_GE(image_frames_.size(), index);
  return image_frames_[index].get();
}

ImageMetadata* GifDecoder::GetMetadata() {
  return &metadata_;
}

bool GifDecoder::IsAllMetadataComplete() const {
  return impl_->DecodingComplete();
}

bool GifDecoder::IsAllFramesComplete() const {
  return impl_->ImageComplete();
}

bool GifDecoder::IsImageComplete() const {
  return impl_->DecodingComplete();
}

Result GifDecoder::Decode() {
  if (HasError())
    return decode_error_;
  return ProcessDecodeResult(impl_->Decode(false));
}

Result GifDecoder::DecodeImageInfo() {
  if (HasError())
    return decode_error_;
  return ProcessDecodeResult(impl_->Decode(true));
}

bool GifDecoder::HasError() const {
  return decode_error_.error();
}

void GifDecoder::Fail(Result error) {
  decode_error_ = error;
}

Result GifDecoder::ProcessDecodeResult(bool result) {
  if (result)
    return Result::Ok();

  if (HasError())
    return decode_error_;

  return Result::Pending();
}

}  // namespace image
