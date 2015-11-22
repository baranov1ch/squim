#ifndef IMAGE_IMAGE_WRITER_H_
#define IMAGE_IMAGE_WRITER_H_

#include "image/result.h"

namespace image {

class ImageFrame;
class ImageMetadata;

class ImageWriter {
 public:
  virtual void SetMetadata(const ImageMetadata* metadata) = 0;
  virtual Result WriteFrame(ImageFrame* metadata) = 0;
  virtual Result FinishWrite() = 0;

  virtual ~ImageWriter() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_WRITER_H_
