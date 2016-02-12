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

#include "squim/image/image_metadata.h"

#include "squim/base/logging.h"

namespace image {

void ImageMetadata::Holder::AddChunk(io::ChunkPtr chunk) {
  data_.push_back(std::move(chunk));
}

void ImageMetadata::Holder::Freeze() {
  frozen_ = true;
}

ImageMetadata::ImageMetadata() {}

const ImageMetadata::Holder& ImageMetadata::GetHolder(Type type) const {
  return GetHolder(type);
}

ImageMetadata::Holder& ImageMetadata::GetHolder(Type type) {
  switch (type) {
    case Type::kICC:
      return icc_;
    case Type::kEXIF:
      return exif_;
    case Type::kXMP:
      return xmp_;
  }
  NOTREACHED();
  return xmp_;
}

bool ImageMetadata::IsCompleted(Type type) const {
  return GetHolder(type).frozen();
}

bool ImageMetadata::IsAllCompleted() const {
  return IsCompleted(Type::kICC) && IsCompleted(Type::kEXIF) &&
         IsCompleted(Type::kXMP);
}

const io::ChunkList& ImageMetadata::Get(Type type) const {
  return GetHolder(type).data();
}

bool ImageMetadata::Has(Type type) const {
  return GetHolder(type).data().empty();
}

void ImageMetadata::Append(Type type, io::ChunkPtr data) {
  GetHolder(type).AddChunk(std::move(data));
}

void ImageMetadata::Freeze(Type type) {
  GetHolder(type).Freeze();
}

void ImageMetadata::FreezeAll() {
  Freeze(Type::kICC);
  Freeze(Type::kEXIF);
  Freeze(Type::kXMP);
}

bool ImageMetadata::Empty() const {
  return !Has(Type::kICC) && !Has(Type::kEXIF) && !Has(Type::kXMP);
}

}  // namespace image
