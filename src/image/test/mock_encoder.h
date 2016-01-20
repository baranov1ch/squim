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

#ifndef IMAGE_MOCK_ENCODER_H_
#define IMAGE_MOCK_ENCODER_H_

#include "image/image_encoder.h"

#include "gmock/gmock.h"

namespace image {

class MockEncoder : public ImageEncoder {
 public:
  MOCK_METHOD1(Initialize, Result(const ImageInfo*));
  MOCK_METHOD2(EncodeFrame, Result(ImageFrame*, bool));
  MOCK_METHOD1(SetMetadata, void(const ImageMetadata*));
  MOCK_METHOD1(FinishWrite, Result(ImageWriter::Stats*));
};

}  // namespace image

#endif  // IMAGE_MOCK_ENCODER_H_
