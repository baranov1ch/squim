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

#include "squim/image/decoding_reader.h"

#include "squim/base/logging.h"
#include "squim/image/image_decoder.h"

namespace image {

DecodingReader::DecodingReader(std::unique_ptr<ImageDecoder> decoder)
    : decoder_(std::move(decoder)) {}

DecodingReader::~DecodingReader() {}

bool DecodingReader::HasMoreFrames() const {
  if (!image_info_read_)
    return true;

  if (!decoder_->IsAllFramesComplete())
    return true;

  if (num_frames_read_ < decoder_->GetFrameCount())
    return true;

  return false;
}

const ImageMetadata* DecodingReader::GetMetadata() const {
  return decoder_->GetMetadata();
}

size_t DecodingReader::GetNumberOfFramesRead() const {
  return num_frames_read_;
}

Result DecodingReader::GetImageInfo(const ImageInfo** info) {
  auto result = AdvanceDecode(true);

  if (result.ok() && info)
    *info = &image_info_;

  return result;
}

Result DecodingReader::GetNextFrame(ImageFrame** frame) {
  auto result = AdvanceDecode(false);

  if (!result.error() && decoder_->IsFrameCompleteAtIndex(num_frames_read_)) {
    if (frame)
      *frame = decoder_->GetFrameAtIndex(num_frames_read_);
    num_frames_read_++;
    return Result::Ok();
  }

  return result;
}

Result DecodingReader::AdvanceDecode(bool header_only) {
  if (!image_info_read_) {
    CHECK(!decoder_->IsImageInfoComplete());
    auto result = decoder_->DecodeImageInfo();
    if (!result.ok())
      return result;

    image_info_ = decoder_->GetImageInfo();
    image_info_read_ = true;
  }

  DCHECK(image_info_read_);
  if (header_only)
    return Result::Ok();

  return decoder_->Decode();
}

Result DecodingReader::GetFrameAtIndex(size_t index, ImageFrame** frame) {
  if (index + 1 > num_frames_read_)
    return Result::Pending();

  CHECK(decoder_->IsFrameCompleteAtIndex(index));
  CHECK(frame);
  *frame = decoder_->GetFrameAtIndex(index);
  return Result::Ok();
}

Result DecodingReader::ReadTillTheEnd() {
  while (!decoder_->IsImageComplete()) {
    auto result = AdvanceDecode(false);
    if (!result.ok())
      return result;
  }
  return Result::Ok();
}

}  // namespace image
