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

#include "image/codecs/png_decoder.h"

#include <cstring>

extern "C" {
#include <setjmp.h>
}

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/scanline_reader.h"
#include "io/buf_reader.h"

extern "C" {
#include "third_party/libpng/upstream/png.h"
}

namespace image {

// static
PngDecoder::Params PngDecoder::Params::Default() {
  Params params;
  params.allowed_color_schemes.insert(ColorScheme::kRGB);
  params.allowed_color_schemes.insert(ColorScheme::kRGBA);
  params.allowed_color_schemes.insert(ColorScheme::kGrayScale);
  params.allowed_color_schemes.insert(ColorScheme::kGrayScaleAlpha);
  return params;
}

class PngDecoder::Impl {
 public:
  Impl(PngDecoder* decoder) : decoder_(decoder) {
    png_ =
        png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, Failed, Warning);
    info_ = png_create_info_struct(png_);
    png_set_progressive_read_fn(png_, this, HeaderAvailable, RowAvailable,
                                Complete);
  }

  ~Impl() {
    if (png_ && info_)
      // This will zero the pointers.
      png_destroy_read_struct(&png_, &info_, 0);
  }

  bool Decode(bool header_only) {
    header_only_ = header_only;

    if (setjmp(png_jmpbuf(png_))) {
      DCHECK(!error_.ok());
      decoder_->Fail(error_);
      return false;
    }

    uint8_t* out;
    do {
      auto result = decoder_->source()->ReadSome(&out);

      if (result.pending())
        return false;

      if (!result.ok()) {
        decoder_->Fail(Result::FromIoResult(result, false));
        return false;
      }

      png_process_data(png_, info_, reinterpret_cast<png_bytep>(out),
                       result.n());

    } while (header_only_ ? state_ < State::kStartDecompress
                          : state_ < State::kDone);

    return true;
  }

  bool HeaderComplete() const { return state_ > State::kHeader; }

  bool DecodingComplete() const { return state_ == State::kDone; }

 private:
  enum class State {
    kHeader,
    kStartDecompress,
    kDecompress,
    kDone,
  };

  void OnHeaderAvailable() {
    auto width = png_get_image_width(png_, info_);
    auto height = png_get_image_height(png_, info_);

    // Protect against large PNGs. See http://bugzil.la/251381 for more details.
    const unsigned long kMaxPngSize = 1000000ul;
    if (width > kMaxPngSize || height > kMaxPngSize) {
      longjmp(png_jmpbuf(png_), 1);
      return;
    }

    decoder_->set_size(width, height);

    int bit_depth, color_type, interlace_type, compression_type, filter_type;
    png_get_IHDR(png_, info_, &width, &height, &bit_depth, &color_type,
                 &interlace_type, &compression_type, &filter_type);

    // Expand palette colors and grayscale 1-2-4 bit depths to 8bit
    if (color_type == PNG_COLOR_TYPE_PALETTE ||
        (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8))
      png_set_expand(png_);

    png_bytep trns = nullptr;
    int trns_count = 0;
    bool has_trns = false;

    // Expand transparency chunk to fully-fledged alpha channel. Webp does not
    // support alpha-chunks.
    if (png_get_valid(png_, info_, PNG_INFO_tRNS)) {
      png_get_tRNS(png_, info_, &trns, &trns_count, 0);
      has_trns = true;
      png_set_expand(png_);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY) {
      if (!decoder_->params_.color_scheme_allowed(ColorScheme::kGrayScale)) {
        png_set_gray_to_rgb(png_);
        decoder_->set_color_space(ColorScheme::kRGB);
      } else {
        if (!has_trns) {
          decoder_->set_color_space(ColorScheme::kGrayScale);
        } else {
          decoder_->set_color_space(ColorScheme::kGrayScaleAlpha);
        }
      }
    } else if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      if (!decoder_->params_.color_scheme_allowed(
              ColorScheme::kGrayScaleAlpha)) {
        png_set_gray_to_rgb(png_);
        decoder_->set_color_space(ColorScheme::kRGBA);
      } else {
        decoder_->set_color_space(ColorScheme::kGrayScaleAlpha);
      }
    } else if (color_type == PNG_COLOR_TYPE_PALETTE ||
               color_type == PNG_COLOR_TYPE_RGB) {
      if (!has_trns) {
        decoder_->set_color_space(ColorScheme::kRGB);
      } else {
        decoder_->set_color_space(ColorScheme::kRGBA);
      }
    } else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
      decoder_->set_color_space(ColorScheme::kRGBA);
    }

    if (decoder_->GetColorScheme() == ColorScheme::kUnknown) {
      decoder_->Fail(Result::Error(Result::Code::kDecodeError,
                                   "Unsupported color scheme"));
      longjmp(png_jmpbuf(png_), 1);
      return;
    }

    // 16-bit color depth not supported yet.
    if (bit_depth == 16)
      png_set_scale_16(png_);

    // TODO: extract ICC profile.

    /* TODO: gamma correction?
    if (!decoder_->GetMetadata().Has(ImageMetadata::Type::kICC)) {
      const double kInverseGamma = 0.45455;
      const double kDefaultGamma = 2.2;
      double gamma;
      if (png_get_gAMA(png_, info_, &gamma)) {
        const double kMaxGamma = 21474.83;
        if ((gamma <= 0.0) || (gamma > kMaxGamma)) {
          gamma = kInverseGamma;
          png_set_gAMA(png_, info_, gamma);
        }
        png_set_gamma(png_, kDefaultGamma, gamma);
      } else {
        png_set_gamma(png_, kDefaultGamma, kInverseGamma);
      }
    }
    */

    // Tell libpng to send us rows for interlaced pngs.
    if (interlace_type == PNG_INTERLACE_ADAM7) {
      png_set_interlace_handling(png_);
      decoder_->set_is_progressive(true);
    }

    // Update our info now.
    png_read_update_info(png_, info_);
    num_color_channels_ = png_get_channels(png_, info_);

    // TODO: extract other metadata.

    if (header_only_)
      decoder_->source()->UnreadN(png_process_data_pause(png_, 0));

    state_ = State::kStartDecompress;
  }

  void OnRowAvailable(png_bytep row, png_uint_32 row_index, int state) {
    auto* frame = &decoder_->image_frame_;
    if (state_ == State::kStartDecompress) {
      frame->Init(decoder_->GetWidth(), decoder_->GetHeight(),
                  decoder_->GetColorScheme());
      decoder_->frame()->set_status(ImageFrame::Status::kPartial);
      state_ = State::kDecompress;
    }

    if (decoder_->IsProgressive() && !interlace_buffer_) {
      auto size = frame->stride() * frame->height();
      interlace_buffer_.reset(new uint8_t[size]);
    }

    /* libpng comments (here to explain what follows).
     *
     * this function is called for every row in the image. If the
     * image is interlacing, and you turned on the interlace handler,
     * this function will be called for every row in every pass.
     * Some of these rows will not be changed from the previous pass.
     * When the row is not changed, the new_row variable will be NULL.
     * The rows and passes are called in order, so you don't really
     * need the row_num and pass, but I'm supplying them because it
     * may make your life easier.
     */

    // Nothing to do if the row is unchanged, or the row is outside
    // the image bounds: libpng may send extra rows, ignore them to
    // make our lives easier.
    if (!row)
      return;

    auto y = row_index;
    if (y >= decoder_->GetHeight())
      return;

    /* libpng comments (continued).
     *
     * For the non-NULL rows of interlaced images, you must call
     * png_progressive_combine_row() passing in the row and the
     * old row.  You can call this function for NULL rows (it will
     * just return) and for non-interlaced images (it just does the
     * memcpy for you) if it will make the code easier. Thus, you
     * can just do this for all cases:
     *
     *    png_progressive_combine_row(png_ptr, old_row, new_row);
     *
     * where old_row is what was displayed for previous rows. Note
     * that the first pass (pass == 0 really) will completely cover
     * the old row, so the rows do not have to be initialized. After
     * the first pass (and only for interlaced images), you will have
     * to pass the current row, and the function will combine the
     * old row and the new row.
     */
    auto row_buf = row;
    if (interlace_buffer_) {
      row_buf = interlace_buffer_.get() + y * frame->stride();
      png_progressive_combine_row(png_, row_buf, row);
    }

    auto scanline = ScanlineReader(&decoder_->image_frame_).at(y);
    CHECK_EQ(num_color_channels_, scanline.frame()->bpp());
    scanline.WritePixels(row_buf);
  }

  void OnFail(png_const_charp msg) {
    LOG(ERROR) << msg;
    error_ = Result::Error(Result::Code::kDecodeError, msg);
    longjmp(png_jmpbuf(png_), 1);
  }

  void OnComplete() {
    decoder_->frame()->set_status(ImageFrame::Status::kComplete);
    state_ = State::kDone;
  }

  static Impl* Get(png_structp png) {
    return reinterpret_cast<Impl*>(png_get_progressive_ptr(png));
  }

  static PNGAPI void HeaderAvailable(png_structp png, png_infop png_info) {
    Get(png)->OnHeaderAvailable();
  }

  static PNGAPI void RowAvailable(png_structp png,
                                  png_bytep row,
                                  png_uint_32 row_index,
                                  int state) {
    Get(png)->OnRowAvailable(row, row_index, state);
  }

  static PNGAPI void Complete(png_structp png, png_infop png_info) {
    Get(png)->OnComplete();
  }

  static PNGAPI void Failed(png_structp png, png_const_charp msg) {
    Get(png)->OnFail(msg);
  }

  static PNGAPI void Warning(png_structp png, png_const_charp msg) {
    LOG(WARNING) << msg;
  }

  PngDecoder* decoder_;
  State state_ = State::kHeader;
  bool header_only_ = false;
  size_t num_color_channels_ = 0;
  png_structp png_;
  png_infop info_;
  std::unique_ptr<uint8_t[]> interlace_buffer_;
  Result error_ = Result::Ok();
};

PngDecoder::PngDecoder(Params params, std::unique_ptr<io::BufReader> source)
    : source_(std::move(source)), params_(params) {
  impl_ = base::make_unique<Impl>(this);
}

PngDecoder::~PngDecoder() {}

uint32_t PngDecoder::GetWidth() const {
  return width_;
}

uint32_t PngDecoder::GetHeight() const {
  return height_;
}

uint64_t PngDecoder::GetSize() const {
  // TODO:
  return 0;
}

ImageType PngDecoder::GetImageType() const {
  return ImageType::kPng;
}

ColorScheme PngDecoder::GetColorScheme() const {
  return color_scheme_;
}

bool PngDecoder::IsProgressive() const {
  return is_progressive_;
}

bool PngDecoder::IsImageInfoComplete() const {
  return impl_->HeaderComplete();
}

size_t PngDecoder::GetFrameCount() const {
  if (!impl_->DecodingComplete())
    return 0;

  return 1;
}

bool PngDecoder::IsMultiFrame() const {
  return false;
}

uint32_t PngDecoder::GetEstimatedQuality() const {
  return 100;
}

bool PngDecoder::IsFrameCompleteAtIndex(size_t index) const {
  if (index != 0)
    return false;

  return impl_->DecodingComplete();
}

ImageFrame* PngDecoder::GetFrameAtIndex(size_t index) {
  CHECK_EQ(0, index);
  return &image_frame_;
}

ImageMetadata* PngDecoder::GetMetadata() {
  return &metadata_;
}

bool PngDecoder::IsAllMetadataComplete() const {
  return impl_->HeaderComplete();
}

bool PngDecoder::IsAllFramesComplete() const {
  return impl_->DecodingComplete();
}

bool PngDecoder::IsImageComplete() const {
  return impl_->DecodingComplete();
}

Result PngDecoder::Decode() {
  if (HasError())
    return decode_error_;

  if (impl_->DecodingComplete())
    return Result::Ok();

  return ProcessDecodeResult(impl_->Decode(false));
}

Result PngDecoder::DecodeImageInfo() {
  if (HasError())
    return decode_error_;

  if (impl_->HeaderComplete())
    return Result::Ok();

  return ProcessDecodeResult(impl_->Decode(true));
}

bool PngDecoder::HasError() const {
  return decode_error_.error();
}

void PngDecoder::Fail(Result error) {
  decode_error_ = error;
}

Result PngDecoder::ProcessDecodeResult(bool result) {
  if (result)
    return Result::Ok();

  if (HasError())
    return decode_error_;

  return Result::Pending();
}

}  // namespace image
