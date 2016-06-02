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

#ifndef SQUIM_IMAGE_OPTIMIZATION_SKIP_METADATA_READER_H_
#define SQUIM_IMAGE_OPTIMIZATION_SKIP_METADATA_READER_H_

#include <memory>

#include "squim/base/make_noncopyable.h"
#include "squim/image/image_reader.h"

namespace image {

// Do not read anything from the |inner| reader if all frames
// are complete.
class SkipMetadataReader : public ImageReader {
  MAKE_NONCOPYABLE(SkipMetadataReader);

 public:
  SkipMetadataReader(std::unique_ptr<ImageReader> inner);

  bool HasMoreFrames() const override;
  const ImageMetadata* GetMetadata() const override;
  size_t GetNumberOfFramesRead() const override;
  Result GetImageInfo(const ImageInfo** info) override;
  Result GetNextFrame(ImageFrame** frame) override;
  Result GetFrameAtIndex(size_t index, ImageFrame** frame) override;
  Result ReadTillTheEnd() override;

 private:
  std::unique_ptr<ImageReader> inner_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_OPTIMIZATION_SKIP_METADATA_READER_H_
