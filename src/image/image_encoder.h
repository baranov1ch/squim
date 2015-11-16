#ifndef IMAGE_IMAGE_ENCODER_H_
#define IMAGE_IMAGE_ENCODER_H_

#include "image/image_constants.h"

namespace image {

class ImageFrame;

class ImageEncoder {
 public:
  virtual Result EncodeFrame(ImageFrame* frame) = 0;

  virtual ~ImageEncoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_ENCODER_H_
