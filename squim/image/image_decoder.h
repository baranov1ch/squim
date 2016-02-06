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

#ifndef SQUIM_IMAGE_IMAGE_DECODER_H_
#define SQUIM_IMAGE_IMAGE_DECODER_H_

#include "squim/image/image_constants.h"
#include "squim/image/result.h"

namespace image {

class ImageFrame;
struct ImageInfo;
class ImageMetadata;

// Decoder interface.
class ImageDecoder {
 public:
  // Should return true if image header (containing width, height, and stuff)
  // has been parsed and client can call any of the methods above.
  virtual bool IsImageInfoComplete() const = 0;

  // Returns ImageInfo info for image being decoded. returned structure is valid
  // iff IsImageInfoComplete() return true.
  virtual const ImageInfo& GetImageInfo() const = 0;

  // Should return true iff frame header at |index| has been successfully
  // parsed.
  virtual bool IsFrameHeaderCompleteAtIndex(size_t index) const = 0;

  // Should return true iff frame at |index| has been successfully decoded
  // so far.
  virtual bool IsFrameCompleteAtIndex(size_t index) const = 0;

  // Should return non-owned pointer to the decoded image frame. Implementations
  // may assume that |index| refers to a decoded frame - it's up to the client
  // to ensure it using IsFrameCompleteAtIndex().
  virtual ImageFrame* GetFrameAtIndex(size_t index) = 0;

  // Returns number of frames *decoded so far*. 1 does not generally mean that
  // image is single frame, other may not be decoded yet
  virtual size_t GetFrameCount() const = 0;

  // Returns image metadata.
  virtual ImageMetadata* GetMetadata() = 0;

  // Should return true if all metadata has been parsed.
  virtual bool IsAllMetadataComplete() const = 0;

  // Should return true if all frames has been decoded.
  virtual bool IsAllFramesComplete() const = 0;

  // Should return true if the whole image is ready.
  virtual bool IsImageComplete() const = 0;

  // Should decode more if data available.
  virtual Result Decode() = 0;

  // Should try to decode only basic image info (usually from image header).
  virtual Result DecodeImageInfo() = 0;

  // True if the error occurred. Decoding cannot progress after that.
  virtual bool HasError() const = 0;

  virtual ~ImageDecoder() {}
};

}  // namespace image

#endif  // SQUIM_IMAGE_IMAGE_DECODER_H_
