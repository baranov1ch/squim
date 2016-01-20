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

#ifndef IMAGE_IMAGE_ENCODER_H_
#define IMAGE_IMAGE_ENCODER_H_

#include "image/image_constants.h"
#include "image/image_writer.h"
#include "image/result.h"

namespace image {

class ImageFrame;
class ImageInfo;
class ImageMetadata;

// General encoder interface.
class ImageEncoder {
 public:
  virtual Result Initialize(const ImageInfo* image_info) = 0;

  // Encodes single frame. |frame| can be null iff |last_frame| is true.
  virtual Result EncodeFrame(ImageFrame* frame, bool last_frame) = 0;

  // Sets metadata for the image. It can be empty at the beginning, but the
  // encoder can consult it from time to time when it decides that certain
  // type of meta should be written.
  virtual void SetMetadata(const ImageMetadata* metadata) = 0;

  // Writes the rest of the stuff (metadata) or flushes output if necessary.
  virtual Result FinishWrite(ImageWriter::Stats* stats) = 0;

  virtual ~ImageEncoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_ENCODER_H_
