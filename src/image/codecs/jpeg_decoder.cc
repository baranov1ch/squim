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
 *
 * The code in this file is very much inspired by blink JPEGImageDecoder.
 */

#include "image/codecs/jpeg_decoder.h"

#include <cstring>

extern "C" {
#include <setjmp.h>
#include <stdio.h>  // jpeglib.h needs stdio FILE.
}

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/scanline_reader.h"
#include "io/buf_reader.h"

extern "C" {
#include "third_party/libjpeg_turbo/upstream/jpeglib.h"
}

namespace image {

// static
JpegDecoder::Params JpegDecoder::Params::Default() {
  Params params;
  params.allowed_color_schemes.insert(ColorScheme::kRGB);
  params.allowed_color_schemes.insert(ColorScheme::kRGBA);
  params.allowed_color_schemes.insert(ColorScheme::kGrayScale);
  params.allowed_color_schemes.insert(ColorScheme::kGrayScaleAlpha);
  return params;
}

class JpegDecoder::Impl {
  MAKE_NONCOPYABLE(Impl);

 public:
  Impl(JpegDecoder* decoder) : decoder_(decoder) {
    memset(&decompress_, 0, sizeof(jpeg_decompress_struct));
    memset(&jpeg_source_, 0, sizeof(DecoderSource));
    memset(&error_handler_, 0, sizeof(DecoderErrorHandler));

    decompress_.err = jpeg_std_error(&error_handler_.pub);
    error_handler_.pub.error_exit = ErrorExit;
    error_handler_.pub.emit_message = EmitMessage;
    error_handler_.decoder = this;

    jpeg_create_decompress(&decompress_);

    DCHECK(!decompress_.src);
    decompress_.src = reinterpret_cast<jpeg_source_mgr*>(&jpeg_source_);

    jpeg_source_.pub.init_source = InitSource;
    jpeg_source_.pub.fill_input_buffer = FillInputBuffer;
    jpeg_source_.pub.skip_input_data = SkipInputData;
    jpeg_source_.pub.resync_to_restart = jpeg_resync_to_restart;  // Default.
    jpeg_source_.pub.term_source = TermSource;
    jpeg_source_.decoder = this;

    const unsigned int kMaxMarkerLength = 0xffff;
    // Exif/XMP.
    jpeg_save_markers(&decompress_, JPEG_APP0 + 1, kMaxMarkerLength);
    // ICC profile.
    jpeg_save_markers(&decompress_, JPEG_APP0 + 2, kMaxMarkerLength);
  }

  ~Impl() {
    decompress_.src = nullptr;
    jpeg_destroy_decompress(&decompress_);
  }

  bool ImageComplete() const { return state_ >= State::kFinish; }

  bool HeaderComplete() const { return state_ > State::kHeader; }

  bool DecodingComplete() const { return state_ == State::kDone; }

  bool Decode(bool header_only) {
    if (setjmp(error_handler_.setjmp_buffer)) {
      decoder_->Fail(error_);
      return false;
    }

    switch (state_) {
      case State::kHeader:
        if (jpeg_read_header(&decompress_, true) == JPEG_SUSPENDED)
          return false;  // I/O suspension.

        switch (decompress_.jpeg_color_space) {
          case JCS_YCbCr:
            if (decoder_->params_.color_scheme_allowed(ColorScheme::kYUV)) {
              decompress_.out_color_space = JCS_YCbCr;
              decoder_->set_color_space(ColorScheme::kYUV);
            } else {
              decompress_.out_color_space = JCS_RGB;
              decoder_->set_color_space(ColorScheme::kRGB);
            }
          case JCS_RGB:
            decompress_.out_color_space = JCS_RGB;
            decoder_->set_color_space(ColorScheme::kRGB);
            break;
          case JCS_GRAYSCALE:
            if (decoder_->params_.color_scheme_allowed(
                    ColorScheme::kGrayScale)) {
              decompress_.out_color_space = JCS_GRAYSCALE;
              decoder_->set_color_space(ColorScheme::kGrayScale);
            } else {
              decompress_.out_color_space = JCS_RGB;
              decoder_->set_color_space(ColorScheme::kRGB);
            }
            break;
          case JCS_CMYK:
          case JCS_YCCK:
            // TODO: do something (Manual conversion).
            decompress_.out_color_space = JCS_CMYK;
          // FALLTHROUGH. CMYK/YCCK not supported yet.
          default:
            decoder_->set_color_space(ColorScheme::kUnknown);
            decoder_->Fail(Result::Error(Result::Code::kDecodeError,
                                         "Unsupported color scheme"));
            return false;
        }

        state_ = State::kStartDecompress;

        decoder_->set_size(decompress_.image_width, decompress_.image_height);
        decoder_->set_is_progressive(decompress_.progressive_mode);
        for (auto marker = decompress_.marker_list; marker;
             marker = marker->next) {
          // TODO: get metadata.
        }

        if (header_only) {
          restart_needed_ = true;
          UpdateRestartPosition();
          ClearBuffer();
          return true;
        }

      // TODO: Optional rescaling when targeting low-end clients:
      //
      // decompress_.scale_num = ....;
      // decompress_.scale_denom = scaleDenominator;
      // jpeg_calc_output_dimensions(&decoder_);

      // Fall through:
      case State::kStartDecompress:
        if (!jpeg_start_decompress(&decompress_))
          return false;  // I/O suspension.

        decoder_->image_frame_.Init(decoder_->GetWidth(), decoder_->GetHeight(),
                                    decoder_->GetColorScheme());

        state_ = decompress_.buffered_image ? State::kDecompressSequential
                                            : State::kDecompressProgressive;

      // Fall through:
      case State::kDecompressSequential:
      // TODO: do we need some progressive decoding? Prolly not.
      case State::kDecompressProgressive:
        if (!rows_) {
          rows_.reset(new uint8_t*[decompress_.output_height]);
          ScanlineReader scanlines(decoder_->frame());
          size_t i = 0;
          for (auto it = scanlines.begin(); it != scanlines.end(); ++it, ++i) {
            rows_[i] = (*it).ptr();
          }
          decoder_->frame()->set_status(ImageFrame::Status::kPartial);
        }

        while (decompress_.output_scanline < decompress_.output_height) {
          int rows_read = jpeg_read_scanlines(
              &decompress_, rows_.get() + decompress_.output_scanline,
              decompress_.output_height - decompress_.output_scanline);
          if (rows_read < 1)
            return false;  // I/O suspension.
        }

        state_ = State::kFinish;
        decoder_->frame()->set_status(ImageFrame::Status::kComplete);

      // Fall through:
      case State::kFinish:
        CHECK_EQ(decompress_.output_height, decompress_.output_scanline);
        if (!jpeg_finish_decompress(&decompress_))
          return false;  // I/O suspension.

        state_ = State::kDone;

      // Fall through:
      case State::kDone:
        break;
    }

    return true;
  }

 private:
  enum class State {
    kHeader,
    kStartDecompress,
    kDecompressProgressive,
    kDecompressSequential,
    kFinish,
    kDone,
  };

  struct DecoderErrorHandler {
    struct jpeg_error_mgr pub;
    int num_corrupt_warnings;
    jmp_buf setjmp_buffer;
    Impl* decoder;
  };

  struct DecoderSource {
    struct jpeg_source_mgr pub;
    Impl* decoder;
  };

  static void InitSource(j_decompress_ptr jd) {}
  static boolean FillInputBuffer(j_decompress_ptr jd) {
    auto* src = reinterpret_cast<DecoderSource*>(jd->src);
    return src->decoder->FillBuffer();
  }

  static void SkipInputData(j_decompress_ptr jd, long num_bytes) {
    auto* src = reinterpret_cast<DecoderSource*>(jd->src);
    src->decoder->SkipBytes(num_bytes);
  }

  static void TermSource(j_decompress_ptr jd) {}

  static void ErrorExit(j_common_ptr cinfo) {
    auto* err = reinterpret_cast<DecoderErrorHandler*>(cinfo->err);
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    LOG(ERROR) << buffer;
    err->decoder->error_ = Result::Error(Result::Code::kDecodeError, buffer);
    longjmp(err->setjmp_buffer, 1);
  }

  static void EmitMessage(j_common_ptr cinfo, int msg_level) {
    if (msg_level > 0 && !VLOG_IS_ON(msg_level))
      return;

    if (msg_level < 0) {
      // It's a warning message.  Since corrupt files may generate many
      // warnings,
      // the policy implemented here is to show only the first warning,
      // unless trace_level >= 3 (as in default libjpeg emitter).
      auto* err = cinfo->err;
      if (err->num_warnings == 0 || err->trace_level >= 3) {
        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);

        if (msg_level == 0) {
          LOG(INFO) << buffer;
        } else {
          LOG(WARNING) << buffer;
        }
      }

      err->num_warnings++;
    } else {
      char buffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, buffer);
      VLOG(msg_level) << buffer;
    }
  }

  void SkipBytes(long num_bytes) {
    if (num_bytes <= 0)
      return;

    auto to_skip = static_cast<size_t>(num_bytes);
    if (to_skip < decompress_.src->bytes_in_buffer) {
      decompress_.src->bytes_in_buffer -= to_skip;
      decompress_.src->next_input_byte += to_skip;
    } else {
      wanted_offset_ += num_bytes - decompress_.src->bytes_in_buffer;
      decompress_.src->bytes_in_buffer = 0;
      decompress_.src->next_input_byte = nullptr;
    }

    restart_position_ =
        decoder_->source()->offset() - decompress_.src->bytes_in_buffer;
    last_set_byte_ = decompress_.src->next_input_byte;
  }

  bool FillBuffer() {
    if (restart_needed_) {
      restart_needed_ = false;
      decoder_->source()->UnreadN(decoder_->source()->offset() -
                                  restart_position_);
    } else {
      UpdateRestartPosition();
    }

    uint8_t* out;
    size_t len;
    do {
      auto result = decoder_->source()->ReadSome(&out);
      if (result.pending()) {
        restart_needed_ = true;
        ClearBuffer();
        return false;
      }

      if (!result.ok()) {
        // TODO: Log?
        decoder_->Fail(Result::FromIoResult(result, false));
        return false;
      }

      auto decrement = std::min(wanted_offset_, result.n());
      wanted_offset_ -= decrement;
      out += decrement;
      len = result.n() - decrement;
    } while (wanted_offset_ > 0 || len == 0);

    decompress_.src->bytes_in_buffer = len;
    auto next_byte = reinterpret_cast<const JOCTET*>(out);
    decompress_.src->next_input_byte = next_byte;
    last_set_byte_ = next_byte;
    return true;
  }

  void UpdateRestartPosition() {
    if (last_set_byte_ != decompress_.src->next_input_byte) {
      // next_input_byte was updated by jpeg, meaning that it found a restart
      // position.
      restart_position_ =
          decoder_->source()->offset() - decompress_.src->bytes_in_buffer;
    }
  }

  void ClearBuffer() {
    decompress_.src->bytes_in_buffer = 0;
    decompress_.src->next_input_byte = nullptr;
    last_set_byte_ = nullptr;
  }

  jpeg_decompress_struct decompress_;
  DecoderErrorHandler error_handler_;
  DecoderSource jpeg_source_;
  State state_ = State::kHeader;
  size_t num_read_ = 0;
  size_t wanted_offset_ = 0;
  JpegDecoder* decoder_;
  std::unique_ptr<uint8_t* []> rows_;
  Result error_ = Result::Ok();
  size_t restart_position_ = 0;
  const JOCTET* last_set_byte_ = nullptr;
  bool restart_needed_ = false;
};

JpegDecoder::JpegDecoder(Params params, std::unique_ptr<io::BufReader> source)
    : source_(std::move(source)), params_(params) {
  impl_ = base::make_unique<Impl>(this);
}

JpegDecoder::~JpegDecoder() {}

uint32_t JpegDecoder::GetWidth() const {
  return width_;
}

uint32_t JpegDecoder::GetHeight() const {
  return height_;
}

uint64_t JpegDecoder::GetSize() const {
  // TODO:
  return 0;
}

ImageType JpegDecoder::GetImageType() const {
  return ImageType::kJpeg;
}

ColorScheme JpegDecoder::GetColorScheme() const {
  return color_scheme_;
}

bool JpegDecoder::IsProgressive() const {
  return is_progressive_;
}

bool JpegDecoder::IsImageInfoComplete() const {
  return impl_->HeaderComplete();
}

size_t JpegDecoder::GetFrameCount() const {
  if (!impl_->DecodingComplete())
    return 0;

  return 1;
}

bool JpegDecoder::IsMultiFrame() const {
  return false;
}

uint32_t JpegDecoder::GetEstimatedQuality() const {
  // TODO:
  return 0;
}

bool JpegDecoder::IsFrameCompleteAtIndex(size_t index) const {
  if (index != 0)
    return false;

  return impl_->DecodingComplete();
}

ImageFrame* JpegDecoder::GetFrameAtIndex(size_t index) {
  CHECK_EQ(0, index);
  return &image_frame_;
}

ImageMetadata* JpegDecoder::GetMetadata() {
  return &metadata_;
}

bool JpegDecoder::IsAllMetadataComplete() const {
  return impl_->DecodingComplete();
}

bool JpegDecoder::IsAllFramesComplete() const {
  return impl_->ImageComplete();
}

bool JpegDecoder::IsImageComplete() const {
  return impl_->ImageComplete();
}

Result JpegDecoder::Decode() {
  if (HasError())
    return decode_error_;
  return ProcessDecodeResult(impl_->Decode(false));
}

Result JpegDecoder::DecodeImageInfo() {
  if (HasError())
    return decode_error_;
  return ProcessDecodeResult(impl_->Decode(true));
}

bool JpegDecoder::HasError() const {
  return decode_error_.error();
}

void JpegDecoder::Fail(Result error) {
  decode_error_ = error;
}

Result JpegDecoder::ProcessDecodeResult(bool result) {
  if (result)
    return Result::Ok();

  if (HasError())
    return decode_error_;

  return Result::Pending();
}

}  // namespace image
