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

#ifndef IMAGE_MOCK_DECODER_H_
#define IMAGE_MOCK_DECODER_H_

#include "image/image_decoder.h"

#include "gmock/gmock.h"

namespace image {

class MockDecoder : public ImageDecoder {
 public:
  MOCK_CONST_METHOD0(IsImageInfoComplete, bool());
  MOCK_CONST_METHOD0(GetImageInfo, const ImageInfo&());
  MOCK_CONST_METHOD1(IsFrameHeaderCompleteAtIndex, bool(size_t));
  MOCK_CONST_METHOD1(IsFrameCompleteAtIndex, bool(size_t));
  MOCK_METHOD1(GetFrameAtIndex, ImageFrame*(size_t));
  MOCK_CONST_METHOD0(GetFrameCount, size_t());
  MOCK_METHOD0(GetMetadata, ImageMetadata*());
  MOCK_CONST_METHOD0(IsAllMetadataComplete, bool());
  MOCK_CONST_METHOD0(IsAllFramesComplete, bool());
  MOCK_CONST_METHOD0(IsImageComplete, bool());
  MOCK_METHOD0(Decode, Result());
  MOCK_METHOD0(DecodeImageInfo, Result());
  MOCK_CONST_METHOD0(HasError, bool());
};

}  // namespace image

#endif  // IMAGE_MOCK_DECODER_H_
