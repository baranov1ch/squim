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

#ifndef SQUIM_IMAGE_IMAGE_METADATA_H_
#define SQUIM_IMAGE_IMAGE_METADATA_H_

#include <map>

#include "squim/base/make_noncopyable.h"
#include "squim/io/chunk.h"

namespace image {

class ImageMetadata {
  MAKE_NONCOPYABLE(ImageMetadata);

 public:
  // Supported types of image metadata.
  enum class Type { kICC, kEXIF, kXMP };

  ImageMetadata();

  bool IsCompleted(Type type) const;
  bool IsAllCompleted() const;
  const io::ChunkList& Get(Type type) const;
  bool Has(Type type) const;

  void Append(Type type, io::ChunkPtr data);
  void Freeze(Type type);
  void FreezeAll();
  bool Empty() const;

 private:
  class Holder {
    MAKE_NONCOPYABLE(Holder);

   public:
    Holder() {}

    void AddChunk(io::ChunkPtr chunk);
    void Freeze();

    const io::ChunkList& data() const { return data_; }
    bool frozen() const { return frozen_; }

   private:
    io::ChunkList data_;
    bool frozen_ = false;
  };

  Holder& GetHolder(Type type);
  const Holder& GetHolder(Type type) const;

  Holder exif_;
  Holder icc_;
  Holder xmp_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_IMAGE_METADATA_H_
