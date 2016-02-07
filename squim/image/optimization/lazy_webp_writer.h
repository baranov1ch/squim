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

#ifndef SQUIM_IMAGE_OPTIMIZATION_LAZY_WEBP_WRITER_H_
#define SQUIM_IMAGE_OPTIMIZATION_LAZY_WEBP_WRITER_H_

#include <memory>

#include "squim/image/image_writer.h"

namespace io {
class VectorWriter;
}

namespace image {

class ImageCodecFactory;
struct ImageInfo;
class ImageMetadata;

// Writer that delays WebP encoder creation until first image frame is ready.
// This gives the opportunity for the higher-level writers to analyze the image
// and prepare to tweak yet-to-be-created encoder params based on that analysis.
class LazyWebPWriter : public ImageWriter {
 public:
  LazyWebPWriter(std::unique_ptr<io::VectorWriter> dest,
                 ImageCodecFactory* codec_factory,
                 const ImageInfo* image_info);

  Result Initialize(const ImageInfo* image_info) override;
  void SetMetadata(const ImageMetadata* metadata) override;
  Result WriteFrame(ImageFrame* frame) override;
  Result FinishWrite(ImageOptimizationStats* stats) override;

 private:
  std::unique_ptr<io::VectorWriter> dest_;
  ImageCodecFactory* codec_factory_;
  const ImageInfo* image_info_;
  const ImageMetadata* image_metadata_ = nullptr;

  std::unique_ptr<ImageWriter> inner_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_OPTIMIZATION_LAZY_WEBP_WRITER_H_
