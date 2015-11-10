#ifndef IMAGE_IMAGE_ENCODER_H_
#define IMAGE_IMAGE_ENCODER_H_

namespace image {

class ImageEncoder {
 public:
  virtual Result EncodeFrame(ImageFrame* frame) = 0;

  virtual ~ImageEncoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_ENCODER_H_
