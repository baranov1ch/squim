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

#ifndef SQUIM_IMAGE_DECODING_READER_H_
#define SQUIM_IMAGE_DECODING_READER_H_

#include <memory>

#include "squim/image/image_info.h"
#include "squim/image/image_reader.h"

namespace image {

class ImageDecoder;

// Reader that simply wraps a decoder to read image from it.
class DecodingReader : public ImageReader {
 public:
  DecodingReader(std::unique_ptr<ImageDecoder> decoder);
  ~DecodingReader() override;

  // ImageReader implementation:
  bool HasMoreFrames() const override;
  const ImageMetadata* GetMetadata() const override;
  size_t GetNumberOfFramesRead() const override;
  Result GetImageInfo(const ImageInfo** info) override;
  Result GetNextFrame(ImageFrame** frame) override;
  Result GetFrameAtIndex(size_t index, ImageFrame** frame) override;
  Result ReadTillTheEnd() override;

 private:
  Result AdvanceDecode(bool header_only);

  // Number of frames already read from the decoder.
  size_t num_frames_read_ = 0;

  // True if decoder has completed image header.
  bool image_info_read_ = false;
  ImageInfo image_info_;
  std::unique_ptr<ImageDecoder> decoder_;
};

}  // namespace image

#endif  // SQUIM_IMAGE_DECODING_READER_H_
