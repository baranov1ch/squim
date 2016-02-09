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

#include "squim/image/codecs/jpeg_decoder.h"

#include <cstring>

extern "C" {
#include <setjmp.h>
#include <stdio.h>  // jpeglib.h needs stdio FILE.
}

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/scanline_reader.h"
#include "squim/io/buf_reader.h"

extern "C" {
#include "third_party/libjpeg_turbo/upstream/jpeglib.h"
}

namespace image {

namespace {

// Following two function are taken as is from mod_pagespeed
// https://github.com/pagespeed/mod_pagespeed/blob/master/pagespeed/kernel/image/jpeg_utils.cc
static double ComputeQualityEntriesSum(JQUANT_TBL* quantization_table,
                                       const unsigned int* std_table) {
  double quality_entries_sum = 0;

  // Quality is defined in terms of the base quantization tables used by
  // encoder. Q = quant table, q = compression quality  and S = table used by
  // encoder, Encoder does the following.
  // if q > 0.5 then Q = 2 - 2*q*S otherwise Q = (0.5/q)*S.
  //
  // Refer 'jpeg_add_quant_table (...)' in jcparam.c for more details.
  //
  // Since we dont have access to the table used by encoder. But it is generally
  // close to the standard table defined by JPEG. Hence, we apply inverse
  // function of the above to using standard table and compute the input image
  // jpeg quality.
  for (int i = 0; i < DCTSIZE2; i++) {
    if (quantization_table->quantval[i] == 1) {
      // 1 is the minimum denominator allowed for any value in the quantization
      // matrix and it implies that quality is set 100.
      quality_entries_sum += 1;
    } else {
      double scale_factor =
          static_cast<double>(quantization_table->quantval[i]) / std_table[i];
      quality_entries_sum +=
          (scale_factor > 1.0 ? (0.5 / scale_factor)
                              : ((2.0 - scale_factor) / 2.0));
    }
  }

  return quality_entries_sum;
}

uint32_t GetJpegQuality(jpeg_decompress_struct* decompress) {
  // The standard tables are taken from JPEG spec section K.1.
  static const unsigned int kStdLuminanceQuantTbl[DCTSIZE2] = {
      16, 11, 10, 16, 24,  40,  51,  61,  12, 12, 14, 19, 26,  58,  60,  55,
      14, 13, 16, 24, 40,  57,  69,  56,  14, 17, 22, 29, 51,  87,  80,  62,
      18, 22, 37, 56, 68,  109, 103, 77,  24, 35, 55, 64, 81,  104, 113, 92,
      49, 64, 78, 87, 103, 121, 120, 101, 72, 92, 95, 98, 112, 100, 103, 99};
  static const unsigned int kStdChrominanceQuantTbl[DCTSIZE2] = {
      17, 18, 24, 47, 99, 99, 99, 99, 18, 21, 26, 66, 99, 99, 99, 99,
      24, 26, 56, 99, 99, 99, 99, 99, 47, 66, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
      99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99};

  double quality_entries_sum = 0;
  double quality_entries_count = 0;
  if (decompress->quant_tbl_ptrs[0] != NULL) {
    quality_entries_sum += ComputeQualityEntriesSum(
        decompress->quant_tbl_ptrs[0], kStdLuminanceQuantTbl);
    quality_entries_count += DCTSIZE2;
  }

  if (decompress->quant_tbl_ptrs[1] != NULL) {
    quality_entries_sum += ComputeQualityEntriesSum(
        decompress->quant_tbl_ptrs[1], kStdChrominanceQuantTbl);
    quality_entries_count += DCTSIZE2;
  }

  if (quality_entries_count > 0) {
    // This computed quality is in the form of a fraction, so multiplying with
    // 100 and rounding off to nearest integer.
    double quality = quality_entries_sum * 100 / quality_entries_count;
    return static_cast<uint32_t>(quality + 0.5);
  }

  return ImageFrame::kUnknownQuality;
}

io::ChunkList ExtractICCP(j_decompress_ptr dinfo) {
  static const char kICCPSignature[] = "ICC_PROFILE";
  static const size_t kICCPSignatureLength = sizeof(kICCPSignature);
  static const size_t kICCPSkipLength = kICCPSignatureLength + 2;
  struct ICCPSegment {
    const uint8_t* data;
    size_t data_length;
  };
  size_t expected_count = 0;
  // Key is segment's sequence number [1, 255] for use in reassembly.
  std::map<size_t, ICCPSegment> iccp_segments;
  for (jpeg_saved_marker_ptr marker = dinfo->marker_list; marker != NULL;
       marker = marker->next) {
    if (marker->marker != JPEG_APP0 + 2 ||
        marker->data_length <= kICCPSignatureLength ||
        std::memcmp(marker->data, kICCPSignature, kICCPSignatureLength) != 0)
      continue;

    // ICC_PROFILE\0<seq><count>; 'seq' starts at 1.
    const size_t seq = marker->data[kICCPSignatureLength];
    const size_t count = marker->data[kICCPSignatureLength + 1];
    const size_t segment_size = marker->data_length - kICCPSkipLength;

    if (segment_size == 0 || count == 0 || seq == 0) {
      LOG(ERROR) << "[ICCP] size (" << segment_size << ") / count (" << seq
                 << ") / sequence number (" << count << ") cannot be 0!";
      return io::ChunkList();
    }

    if (expected_count == 0) {
      expected_count = count;
    } else if (count != expected_count) {
      LOG(ERROR) << "[ICCP] Inconsistent segment count (" << expected_count
                 << " / " << count << ")!";
      return io::ChunkList();
    }

    if (iccp_segments.find(seq) != iccp_segments.end()) {
      LOG(ERROR) << "[ICCP] Duplicate segment number (" << seq << ")!";
      return io::ChunkList();
    }

    ICCPSegment segment{marker->data + kICCPSignatureLength, segment_size};
    iccp_segments[seq] = segment;
  }

  if (iccp_segments.empty())
    return io::ChunkList();

  if (iccp_segments.size() != iccp_segments.rbegin()->first) {
    LOG(ERROR) << "[ICCP] Discontinuous segments, expected: "
               << iccp_segments.size()
               << " actual: " << iccp_segments.rbegin()->first << "!";
    return io::ChunkList();
  }

  if (iccp_segments.size() != expected_count) {
    LOG(ERROR) << "[ICCP] Segment count: " << iccp_segments.size()
               << " does not match expected: " << expected_count << "!";
    return io::ChunkList();
  }

  io::ChunkList ret;
  for (const auto& kv : iccp_segments) {
    ret.push_back(io::Chunk::Copy(kv.second.data, kv.second.data_length));
  }

  return ret;
}

}  // namespace

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
      case State::kHeader: {
        if (jpeg_read_header(&decompress_, true) == JPEG_SUSPENDED)
          return false;  // I/O suspension.

        auto* frame = decoder_->frame();
        switch (decompress_.jpeg_color_space) {
          case JCS_YCbCr:
            if (decoder_->params_.color_scheme_allowed(ColorScheme::kYUV)) {
              decompress_.out_color_space = JCS_YCbCr;
              frame->set_color_scheme(ColorScheme::kYUV);
            } else {
              decompress_.out_color_space = JCS_RGB;
              frame->set_color_scheme(ColorScheme::kRGB);
            }
          case JCS_RGB:
            decompress_.out_color_space = JCS_RGB;
            frame->set_color_scheme(ColorScheme::kRGB);
            break;
          case JCS_GRAYSCALE:
            if (decoder_->params_.color_scheme_allowed(
                    ColorScheme::kGrayScale)) {
              decompress_.out_color_space = JCS_GRAYSCALE;
              frame->set_color_scheme(ColorScheme::kGrayScale);
            } else {
              decompress_.out_color_space = JCS_RGB;
              frame->set_color_scheme(ColorScheme::kRGB);
            }
            break;
          case JCS_CMYK:
          case JCS_YCCK:
            // TODO: do something (Manual conversion).
            decompress_.out_color_space = JCS_CMYK;
          // FALLTHROUGH. CMYK/YCCK not supported yet.
          default:
            frame->set_color_scheme(ColorScheme::kUnknown);
            decoder_->Fail(Result::Error(Result::Code::kDecodeError,
                                         "Unsupported color scheme"));
            return false;
        }

        state_ = State::kStartDecompress;

        frame->set_size(decompress_.image_width, decompress_.image_height);
        decoder_->image_info_.width = decompress_.image_width;
        decoder_->image_info_.height = decompress_.image_height;
        frame->set_is_progressive(decompress_.progressive_mode);
        for (auto marker = decompress_.marker_list; marker;
             marker = marker->next) {
          // TODO: get metadata.
        }
        frame->set_quality(GetJpegQuality(&decompress_));

        frame->set_status(ImageFrame::Status::kHeaderComplete);

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
      }
      // Fall through:
      case State::kStartDecompress:
        if (!jpeg_start_decompress(&decompress_))
          return false;  // I/O suspension.

        decoder_->frame()->Init();
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

        ExtractMetadata();
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

  void ExtractMetadata() {
    static const struct Metadata {
      int marker;
      const char* signature;
      size_t signature_length;
      ImageMetadata::Type type;
    } kJPEGMetadataMap[] = {
        // Exif 2.2 Section 4.7.2 Interoperability Structure of APP1 ...
        {JPEG_APP0 + 1, "Exif\0", 6, ImageMetadata::Type::kEXIF},
        // XMP Specification Part 3 Section 3 Embedding XMP Metadata ... #JPEG
        // TODO(jzern) Add support for 'ExtendedXMP'
        {JPEG_APP0 + 1, "http://ns.adobe.com/xap/1.0/", 29,
         ImageMetadata::Type::kXMP},
    };

    auto dinfo = reinterpret_cast<j_decompress_ptr>(&decompress_);

    auto chunks = ExtractICCP(dinfo);
    while (!chunks.empty()) {
      auto chunk = std::move(chunks.front());
      decoder_->metadata_.Append(ImageMetadata::Type::kICC, std::move(chunk));
      chunks.pop_front();
    }

    for (jpeg_saved_marker_ptr marker = dinfo->marker_list; marker != nullptr;
         marker = marker->next) {
      auto meta_it =
          std::find_if(std::begin(kJPEGMetadataMap), std::end(kJPEGMetadataMap),
                       [marker](const Metadata& e) {
                         return marker->marker == e.marker &&
                                marker->data_length > e.signature_length &&
                                std::memcmp(marker->data, e.signature,
                                            e.signature_length) == 0;
                       });

      if (meta_it == std::end(kJPEGMetadataMap))
        continue;

      if (decoder_->metadata_.Has(meta_it->type)) {
        LOG(WARNING) << "Ignoring additional '"
                     << base::StringPiece(meta_it->signature,
                                          meta_it->signature_length)
                     << "'";
        continue;
      }

      auto chunk = io::Chunk::View(marker->data, marker->data_length)
                       ->Slice(meta_it->signature_length)
                       ->Clone();
      decoder_->metadata_.Append(meta_it->type, std::move(chunk));
    }

    decoder_->metadata_.FreezeAll();
  }

  jpeg_decompress_struct decompress_;
  DecoderErrorHandler error_handler_;
  DecoderSource jpeg_source_;
  State state_ = State::kHeader;
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
  image_info_.type = ImageType::kJpeg;
}

JpegDecoder::~JpegDecoder() {}

bool JpegDecoder::IsImageInfoComplete() const {
  return impl_->HeaderComplete();
}

const ImageInfo& JpegDecoder::GetImageInfo() const {
  return image_info_;
}

bool JpegDecoder::IsFrameHeaderCompleteAtIndex(size_t index) const {
  return index == 0 && impl_->HeaderComplete();
}

bool JpegDecoder::IsFrameCompleteAtIndex(size_t index) const {
  return index == 0 && impl_->DecodingComplete();
}

ImageFrame* JpegDecoder::GetFrameAtIndex(size_t index) {
  CHECK_EQ(0, index);
  return &image_frame_;
}

size_t JpegDecoder::GetFrameCount() const {
  return impl_->DecodingComplete() ? 1 : 0;
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
