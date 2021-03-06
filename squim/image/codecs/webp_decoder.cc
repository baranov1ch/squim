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

#include "squim/image/codecs/webp_decoder.h"

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "google/libwebp/upstream/src/webp/decode.h"
#include "google/libwebp/upstream/src/webp/demux.h"
#include "squim/image/scanline_reader.h"
#include "squim/io/buf_reader.h"

namespace image {

// static
WebPDecoder::Params WebPDecoder::Params::Default() {
  Params params;
  params.allowed_color_schemes.insert(ColorScheme::kYUV);
  params.allowed_color_schemes.insert(ColorScheme::kYUVA);
  return params;
}

class WebPDecoder::Impl {
  MAKE_NONCOPYABLE(Impl);

 public:
  Impl(WebPDecoder* decoder) : decoder_(decoder) {}
  ~Impl() {}

  bool Decode(bool header_only) { return true; }

  bool HeaderComplete() const { return true; }

  bool ImageComplete() const { return true; }

  bool DecodingComplete() const { return true; }

 private:
  void UpdateDemuxer() {}

  WebPIDecoder* webp_decoder_;
  WebPDecBuffer decoder_buffer_;

  WebPDecoder* decoder_;
};

WebPDecoder::WebPDecoder(Params params, std::unique_ptr<io::BufReader> source)
    : source_(std::move(source)), params_(params) {
  impl_ = base::make_unique<Impl>(this);
  image_info_.type = ImageType::kWebP;
}

WebPDecoder::~WebPDecoder() {}

bool WebPDecoder::IsImageInfoComplete() const {
  return impl_->HeaderComplete();
}

const ImageInfo& WebPDecoder::GetImageInfo() const {
  return image_info_;
}

bool WebPDecoder::IsFrameHeaderCompleteAtIndex(size_t index) const {
  if (index >= image_frames_.size())
    return false;

  return image_frames_[index]->status() == ImageFrame::Status::kHeaderComplete;
}

bool WebPDecoder::IsFrameCompleteAtIndex(size_t index) const {
  if (index >= image_frames_.size())
    return false;

  return image_frames_[index]->status() == ImageFrame::Status::kComplete;
}

ImageFrame* WebPDecoder::GetFrameAtIndex(size_t index) {
  CHECK_GE(image_frames_.size(), index);
  return image_frames_[index].get();
}

size_t WebPDecoder::GetFrameCount() const {
  return image_frames_.size();
}

ImageMetadata* WebPDecoder::GetMetadata() {
  return &metadata_;
}

bool WebPDecoder::IsAllMetadataComplete() const {
  return impl_->DecodingComplete();
}

bool WebPDecoder::IsAllFramesComplete() const {
  return impl_->ImageComplete();
}

bool WebPDecoder::IsImageComplete() const {
  return impl_->DecodingComplete();
}

Result WebPDecoder::Decode() {
  if (HasError())
    return decode_error_;
  return ProcessDecodeResult(impl_->Decode(false));
}

Result WebPDecoder::DecodeImageInfo() {
  if (HasError())
    return decode_error_;
  return ProcessDecodeResult(impl_->Decode(true));
}

bool WebPDecoder::HasError() const {
  return decode_error_.error();
}

void WebPDecoder::Fail(Result error) {
  decode_error_ = error;
}

Result WebPDecoder::ProcessDecodeResult(bool result) {
  if (result)
    return Result::Ok();

  if (HasError())
    return decode_error_;

  return Result::Pending();
}

}  // namespace image
