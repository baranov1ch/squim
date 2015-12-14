#ifndef IMAGE_IMAGE_ENCODER_H_
#define IMAGE_IMAGE_ENCODER_H_

#include "image/image_constants.h"
#include "image/result.h"

namespace image {

class ImageFrame;
class ImageMetadata;

// General encoder interface.
class ImageEncoder {
 public:
  // Encodes single frame. |frame| can be null iff |last_frame| is true.
  virtual Result EncodeFrame(ImageFrame* frame, bool last_frame) = 0;

  // Sets metadata for the image. It can be empty at the beginning, but the
  // encoder can consult it from time to time when it decides that certain
  // type of meta should be written.
  virtual void SetMetadata(const ImageMetadata* metadata) = 0;

  // Writes the rest of the stuff (metadata) or flushes output if necessary.
  virtual Result FinishWrite() = 0;

  virtual ~ImageEncoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_ENCODER_H_
