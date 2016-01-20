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

#include "image/single_frame_writer.h"

#include "image/image_encoder.h"

namespace image {

SingleFrameWriter::SingleFrameWriter(std::unique_ptr<ImageEncoder> encoder)
    : encoder_(std::move(encoder)) {}

SingleFrameWriter::~SingleFrameWriter() {}

Result SingleFrameWriter::Initialize(const ImageInfo* image_info) {
  return encoder_->Initialize(image_info);
}

void SingleFrameWriter::SetMetadata(const ImageMetadata* metadata) {
  encoder_->SetMetadata(metadata);
}

Result SingleFrameWriter::WriteFrame(ImageFrame* frame) {
  if (frame_written_)
    return Result::Error(
        Result::Code::kFailed,
        "Attempt to write multiple frames using SingleFrameWriter");

  frame_written_ = true;
  return encoder_->EncodeFrame(frame, true);
}

Result SingleFrameWriter::FinishWrite(Stats* stats) {
  return encoder_->FinishWrite(stats);
}

}  // namespace image
