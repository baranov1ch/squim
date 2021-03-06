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

#include "squim/image/multi_frame_writer.h"

#include "squim/image/image_encoder.h"

namespace image {

MultiFrameWriter::MultiFrameWriter(std::unique_ptr<ImageEncoder> encoder)
    : encoder_(std::move(encoder)) {}

MultiFrameWriter::~MultiFrameWriter() {}

Result MultiFrameWriter::Initialize(const ImageInfo* image_info) {
  return encoder_->Initialize(image_info);
}

void MultiFrameWriter::SetMetadata(const ImageMetadata* metadata) {
  encoder_->SetMetadata(metadata);
}

Result MultiFrameWriter::WriteFrame(ImageFrame* frame) {
  return encoder_->EncodeFrame(frame, false);
}

Result MultiFrameWriter::FinishWrite(ImageOptimizationStats* stats) {
  auto result = encoder_->EncodeFrame(nullptr, true);

  // Normally pending should result in another call to this function but who
  // cares, finish writes up-to date does nothing which may suspend IO. If that
  // ever happens, add another step.
  if (result.error())
    return result;

  return encoder_->FinishWrite(stats);
}

}  // namespace image
