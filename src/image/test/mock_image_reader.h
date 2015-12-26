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

#ifndef IMAGE_MOCK_IMAGE_READER_H_
#define IMAGE_MOCK_IMAGE_READER_H_

#include "image/image_reader.h"

#include "image/image_info.h"

#include "gmock/gmock.h"

namespace image {

class MockImageReader : public ImageReader {
 public:
  MOCK_CONST_METHOD0(HasMoreFrames, bool());
  MOCK_CONST_METHOD0(GetMetadata, const ImageMetadata*());
  MOCK_CONST_METHOD0(GetNumberOfFramesRead, size_t());
  MOCK_METHOD1(GetImageInfo, Result(const ImageInfo**));
  MOCK_METHOD1(GetNextFrame, Result(ImageFrame**));
  MOCK_METHOD2(GetFrameAtIndex, Result(size_t, ImageFrame**));
  MOCK_METHOD0(ReadTillTheEnd, Result());

  Result GetFakeImageInfo(const ImageInfo** info) {
    *info = &image_info;
    return Result::Ok();
  }

  ImageInfo image_info;
};

}  // namespace image

#endif  // IMAGE_MOCK_IMAGE_READER_H_
