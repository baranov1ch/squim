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

#include "squim/image/codecs/gif_decoder.h"

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/codecs/gif/gif_image_parser.h"
#include "squim/io/buf_reader.h"

namespace image {

namespace {

ImageFrame::DisposalMethod GifDisposalToDisposalMethod(
    GifImage::DisposalMethod gif_disposal) {
  switch (gif_disposal) {
    case GifImage::DisposalMethod::kOverwriteBgcolor:
      return ImageFrame::DisposalMethod::kBackground;
    case GifImage::DisposalMethod::kOverwritePrevious:
      return ImageFrame::DisposalMethod::kRestorePrevious;
    case GifImage::DisposalMethod::kKeep:
    case GifImage::DisposalMethod::kNotSpecified:
      return ImageFrame::DisposalMethod::kNone;
    default:
      NOTREACHED();
      return ImageFrame::DisposalMethod::kNone;
  }
}
}

// static
GifDecoder::Params GifDecoder::Params::Default() {
  Params params;
  params.allowed_color_schemes.insert(ColorScheme::kRGB);
  params.allowed_color_schemes.insert(ColorScheme::kRGBA);
  return params;
}

class GifDecoder::Impl {
  MAKE_NONCOPYABLE(Impl);

 public:
  Impl(GifDecoder* decoder) : decoder_(decoder) {
    gif_parser_ = base::make_unique<GifImage::Parser>(decoder_->source_.get(),
                                                      &gif_image_);
  }

  ~Impl() {}

  bool Decode(bool header_only) {
    auto result = Result::Ok();
    if (header_only) {
      result = gif_parser_->ParseHeader();
    } else {
      result = gif_parser_->Parse();
    }
    if (result.error()) {
      decoder_->Fail(result);
      return false;
    } else if (result.pending()) {
      return false;  // IO suspend.
    }

    if (!header_complete_reported_ && HeaderComplete()) {
      header_complete_reported_ = true;
      decoder_->image_info_.width = gif_image_.screen_width();
      decoder_->image_info_.height = gif_image_.screen_height();
      decoder_->image_info_.multiframe = true;
      const auto* global_color_table = gif_image_.global_color_table();
      auto bg_color_idx = gif_image_.background_color_index();
      if (global_color_table && bg_color_idx < global_color_table->size()) {
        auto color = global_color_table->GetColor(bg_color_idx);
        decoder_->image_info_.bg_color = {
            {color.r(), color.g(), color.b(), 0xFF}};
      }
      decoder_->image_info_.loop_count = gif_image_.loop_count();
    }

    const auto& frames = gif_image_.frames();
    while (num_frames_ready_ < frames.size()) {
      const auto& gif_frame = frames[num_frames_ready_];
      const auto* color_table = gif_frame->GetColorTable();
      if (!color_table) {
        decoder_->Fail(
            Result::Error(Result::Code::kDecodeError, "Missing color table"));
        return false;
      }

      if (num_frames_ready_ == 1 &&
          gif_image_.background_color_index() == GifImage::kNoBackgroundColor) {
        LOG(WARNING) << "No background color for animated image";
        // Fall back to opaque white.
        decoder_->image_info_.bg_color = {{0xFF, 0xFF, 0xFF, 0xFF}};
      }

      auto frame = base::make_unique<ImageFrame>();
      frame->set_offset(gif_frame->x_offset(), gif_frame->y_offset());
      frame->set_size(gif_frame->width(), gif_frame->height());
      frame->set_is_progressive(gif_frame->is_progressive());
      if (gif_frame->transparent_pixel() != GifImage::kNoTransparentPixel) {
        frame->set_color_scheme(ColorScheme::kRGBA);
      } else {
        frame->set_color_scheme(ColorScheme::kRGB);
      }
      frame->set_disposal_method(
          GifDisposalToDisposalMethod(gif_frame->disposal_method()));
      frame->set_duration(gif_frame->duration());
      frame->set_status(ImageFrame::Status::kHeaderComplete);

      frame->Init();
      Bitmap bitmap(frame.get());
      for (auto y = 0; y < gif_frame->height(); ++y) {
        for (auto x = 0; x < gif_frame->width(); ++x) {
          auto idx = gif_frame->GetPixel(x, y);
          if (idx > color_table->size()) {
            decoder_->Fail(Result::Error(Result::Code::kDecodeError,
                                         "Invalid color index"));
            return false;
          }

          auto color = color_table->GetColor(idx);
          if (frame->color_scheme() == ColorScheme::kRGBA) {
            if (idx == gif_frame->transparent_pixel()) {
              bitmap.GetPixel<RGBAPixel>(x, y).set(0xFF, 0xFF, 0xFF, 0x00);
            } else {
              bitmap.GetPixel<RGBAPixel>(x, y)
                  .set(color.r(), color.g(), color.b(), 0xFF);
            }
          } else {
            bitmap.GetPixel<RGBPixel>(x, y)
                .set(color.r(), color.g(), color.b());
          }
        }
      }

      frame->set_status(ImageFrame::Status::kComplete);
      decoder_->image_frames_.push_back(std::move(frame));
      num_frames_ready_++;
    }

    return true;
  }

  bool HeaderComplete() const { return gif_parser_->header_complete(); }

  bool ImageComplete() const { return gif_parser_->complete(); }

  bool DecodingComplete() const { return gif_parser_->complete(); }

  ImageMetadata* GetMetadata() { return gif_image_.GetMetadata(); }

 private:
  GifDecoder* decoder_;
  GifImage gif_image_;
  size_t num_frames_ready_ = 0;
  bool header_complete_reported_ = false;
  std::unique_ptr<GifImage::Parser> gif_parser_;
};

GifDecoder::GifDecoder(Params params, std::unique_ptr<io::BufReader> source)
    : source_(std::move(source)), params_(params) {
  impl_ = base::make_unique<Impl>(this);
  image_info_.type = ImageType::kGif;
}

GifDecoder::~GifDecoder() {}

const ImageInfo& GifDecoder::GetImageInfo() const {
  return image_info_;
}

bool GifDecoder::IsImageInfoComplete() const {
  return impl_->HeaderComplete();
}

bool GifDecoder::IsFrameHeaderCompleteAtIndex(size_t index) const {
  if (index >= image_frames_.size())
    return false;

  return image_frames_[index]->status() == ImageFrame::Status::kHeaderComplete;
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

size_t GifDecoder::GetFrameCount() const {
  return image_frames_.size();
}

ImageMetadata* GifDecoder::GetMetadata() {
  return impl_->GetMetadata();
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
