#include "image/codecs/jpeg_decoder.h"

#include <cstring>

extern "C" {
#include <setjmp.h>
#include <stdio.h>  // jpeglib.h needs stdio FILE.
}

#include "base/memory/make_unique.h"
#include "glog/logging.h"
#include "image/scanline_reader.h"
#include "io/buf_reader.h"

extern "C" {
#include "third_party/libjpeg_turbo/upstream/jpeglib.h"
}

namespace image {

namespace {

enum class State {
  kHeader,
  kStartDecompress,
  kDecompressProgressive,
  kDecompressSequential,
  kDone,
};

}  // namespace

class JpegDecoder::Impl {
  MAKE_NONCOPYABLE(Impl);

 public:
  Impl(JpegDecoder* decoder) : decoder_(decoder) {
    memset(&decompress_, 0, sizeof(jpeg_decompress_struct));

    decompress_.err = jpeg_std_error(&error_handler_.pub);
    error_handler_.pub.error_exit = ErrorExit;

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

  bool HeaderComplete() const { return state_ > State::kHeader; }

  bool DecodingComplete() const { return state_ == State::kDone; }

  bool Decode(bool header_only) {
    if (setjmp(error_handler_.setjmp_buffer)) {
      decoder_->Fail(Result::Error(Result::Code::kDecodeError));
      return false;
    }

    switch (state_) {
      case State::kHeader:
        if (jpeg_read_header(&decompress_, true) == JPEG_SUSPENDED)
          return false;  // I/O suspension.

        switch (decompress_.jpeg_color_space) {
          case JCS_YCbCr:
          case JCS_RGB:
            decompress_.out_color_space = JCS_RGB;
            decoder_->set_color_space(ColorScheme::kRGB);
            break;
          case JCS_GRAYSCALE:
            decompress_.out_color_space = JCS_GRAYSCALE;
            decoder_->set_color_space(ColorScheme::kGrayScale);
            break;
          case JCS_CMYK:
          case JCS_YCCK:
            // TODO: do something (Manual conversion).
            decompress_.out_color_space = JCS_CMYK;
            decoder_->set_color_space(ColorScheme::kCMYK);
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

        if (header_only)
          return true;

      // TODO: Optional rescaling when targeting low-end clients:
      //
      // decompress_.scale_num = ....;
      // decompress_.scale_denom = scaleDenominator;
      // jpeg_calc_output_dimensions(&decoder_);

      // Fall through:
      case State::kStartDecompress:
        if (!jpeg_start_decompress(&decompress_))
          return false;  // I/O suspension.

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
        }

        while (decompress_.output_scanline < decompress_.output_height) {
          int rows_read = jpeg_read_scanlines(
              &decompress_, rows_.get() + decompress_.output_scanline,
              decompress_.output_height - decompress_.output_scanline);
          if (rows_read < 1)
            return false;  // I/O suspension.
        }

        state_ = State::kDone;

      // Fall through:
      case State::kDone:
        CHECK_EQ(decompress_.output_height, decompress_.output_scanline);
        return jpeg_finish_decompress(&decompress_);
    }

    return true;
  }

 private:
  struct DecoderErrorHandler {
    struct jpeg_error_mgr pub;
    int num_corrupt_warnings;
    jmp_buf setjmp_buffer;
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
    longjmp(err->setjmp_buffer, -1);
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
  }

  bool FillBuffer() {
    uint8_t* out;
    size_t len;
    do {
      auto result = decoder_->source()->ReadSome(&out);
      if (result.pending())
        return false;

      if (!result.ok()) {
        decoder_->Fail(Result::FromIoResult(result, false));
        return false;
      }

      auto decrement = std::min(wanted_offset_, result.n());
      wanted_offset_ -= decrement;
      out += decrement;
      len = result.n() - decrement;
    } while (wanted_offset_ > 0);

    if (len <= 0)
      return false;

    decompress_.src->bytes_in_buffer = len;
    auto next_byte = reinterpret_cast<const JOCTET*>(out);
    decompress_.src->next_input_byte = next_byte;
    return true;
  }

  jpeg_decompress_struct decompress_;
  DecoderErrorHandler error_handler_;
  DecoderSource jpeg_source_;
  State state_ = State::kHeader;
  size_t num_read_ = 0;
  size_t wanted_offset_ = 0;
  JpegDecoder* decoder_;
  std::unique_ptr<uint8_t* []> rows_;
};

JpegDecoder::JpegDecoder(std::unique_ptr<io::BufReader> source)
    : source_(std::move(source)) {
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
  return impl_->HeaderComplete();
}

bool JpegDecoder::IsAllFramesComplete() const {
  return impl_->DecodingComplete();
}

bool JpegDecoder::IsImageComplete() const {
  return impl_->DecodingComplete();
}

Result JpegDecoder::Decode() {
  return ProcessDecodeResult(impl_->Decode(false));
}

Result JpegDecoder::DecodeImageInfo() {
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
