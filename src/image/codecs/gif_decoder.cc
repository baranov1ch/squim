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

#include "image/codecs/gif_decoder.h"
#include "image/codecs/gif/gif_image_parser.h"

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "io/buf_reader.h"

namespace image {

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
      decoder_->width_ = gif_image_.screen_width();
      decoder_->height_ = gif_image_.screen_height();
      decoder_->color_scheme_ = ColorScheme::kRGB;
      // is_progressive_ can change over time, since every frame interlaced on
      // its own.
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

      if (!decoder_->is_progressive_ && gif_frame->is_progressive()) {
        decoder_->is_progressive_ = true;
      }

      if (num_frames_ready_ == 0) {
        const auto* global_color_table = gif_image_.global_color_table();
        auto bg_color_idx = gif_image_.background_color_index();
        if (global_color_table && bg_color_idx < global_color_table->size()) {
          auto color = global_color_table->GetColor(bg_color_idx);
          decoder_->bg_color_ = {{color.r(), color.g(), color.b(), 0xFF}};
        }
      } else if (num_frames_ready_ == 1 &&
                 gif_image_.background_color_index() ==
                     GifImage::kNoBackgroundColor) {
        // Just to write warning.
        LOG(WARNING) << "No background color for animated image";
      }

      auto frame = base::make_unique<ImageFrame>();
      frame->set_offset(gif_frame->x_offset(), gif_frame->y_offset());
      if (gif_frame->transparent_pixel() != GifImage::kNoTransparentPixel) {
        frame->Init(gif_frame->width(), gif_frame->height(),
                    ColorScheme::kRGBA);
      } else {
        frame->Init(gif_frame->width(), gif_frame->height(), ColorScheme::kRGB);
      }

      if (gif_frame->disposal_method() ==
          GifImage::DisposalMethod::kOverwriteBgcolor) {
        frame->set_should_dispose_to_background(true);
      } else if (gif_frame->disposal_method() ==
                 GifImage::DisposalMethod::kOverwritePrevious) {
        LOG(WARNING) << "Previous: unsupported frame disposal method";
        frame->set_should_dispose_to_background(true);
      }
      frame->set_duration(gif_frame->duration());

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
