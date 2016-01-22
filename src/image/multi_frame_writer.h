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

#ifndef IMAGE_MULTI_FRAME_WRITER_H_
#define IMAGE_MULTI_FRAME_WRITER_H_

#include <memory>

#include "base/make_noncopyable.h"
#include "image/image_writer.h"

namespace image {

class ImageEncoder;

class MultiFrameWriter : public ImageWriter {
  MAKE_NONCOPYABLE(MultiFrameWriter);

 public:
  MultiFrameWriter(std::unique_ptr<ImageEncoder> encoder);
  ~MultiFrameWriter() override;

  Result Initialize(const ImageInfo* image_info) override;
  void SetMetadata(const ImageMetadata* metadata) override;
  Result WriteFrame(ImageFrame* frame) override;
  Result FinishWrite(Stats* stats) override;

 private:
  std::unique_ptr<ImageEncoder> encoder_;
  bool last_frame_written_ = false;
};

}  // namespace image

#endif  // IMAGE_MULTI_FRAME_WRITER_H_
