#ifndef IMAGE_IMAGE_ENCODER_H_
#define IMAGE_IMAGE_ENCODER_H_

#include "image/image_constants.h"

namespace image {

class ImageFrame;

// General encoder interface.
class ImageEncoder {
 public:
  // Encodes single frame. |frame| can be null iff |last_frame| is true.
  virtual Result EncodeFrame(ImageFrame* frame, bool last_frame) = 0;

  // Sets metadata for the image. It can be empty at the beginning, but the
  // encoder can consult it from time to time when it decides that certain
  // type of meta should be written.
  virtual void SetMetadata(const Metadata* metadata) = 0;

  virtual ~ImageEncoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_ENCODER_H_
