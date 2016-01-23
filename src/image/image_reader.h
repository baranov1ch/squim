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

#ifndef IMAGE_IMAGE_READER_H_
#define IMAGE_IMAGE_READER_H_

#include "image/result.h"

namespace image {

struct ImageInfo;
class ImageFrame;
class ImageMetadata;

class ImageReader {
 public:
  virtual bool HasMoreFrames() const = 0;
  virtual const ImageMetadata* GetMetadata() const = 0;
  virtual size_t GetNumberOfFramesRead() const = 0;

  // |info| can be null. In this case reader should just try to advance
  // underlying decoder to get the info.
  virtual Result GetImageInfo(const ImageInfo** info) = 0;

  // |frame| can be null as well. Same as above.
  virtual Result GetNextFrame(ImageFrame** frame) = 0;

  // Random access to already read frames. |index| must be smaller than
  // GetNumberOfFramesRead(). |frame| must not be null.
  virtual Result GetFrameAtIndex(size_t index, ImageFrame** frame) = 0;

  // Read all the image till the end. Can used to get the information after
  // frames data, e.g. webp exif/xmp metadata chunks are after frames.
  virtual Result ReadTillTheEnd() = 0;

  virtual ~ImageReader() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_READER_H_
