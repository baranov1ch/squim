/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#include "squim/image/optimization/skip_metadata_reader.h"

namespace image {

SkipMetadataReader::SkipMetadataReader(std::unique_ptr<ImageReader> inner)
    : inner_(std::move(inner)) {}

bool SkipMetadataReader::HasMoreFrames() const {
  return inner_->HasMoreFrames();
}

const ImageMetadata* SkipMetadataReader::GetMetadata() const {
  return inner_->GetMetadata();
}

size_t SkipMetadataReader::GetNumberOfFramesRead() const {
  return inner_->GetNumberOfFramesRead();
}

Result SkipMetadataReader::GetImageInfo(const ImageInfo** info) {
  return inner_->GetImageInfo(info);
}

Result SkipMetadataReader::GetNextFrame(ImageFrame** frame) {
  return inner_->GetNextFrame(frame);
}

Result SkipMetadataReader::GetFrameAtIndex(size_t index, ImageFrame** frame) {
  return inner_->GetFrameAtIndex(index, frame);
}

Result SkipMetadataReader::ReadTillTheEnd() {
  while (HasMoreFrames()) {
    auto result = GetNextFrame(nullptr);
    if (!result.ok())
      return result;
  }
  return Result::Ok();
}

}  // namespace image
