#ifndef IMAGE_IMAGE_READER_H_
#define IMAGE_IMAGE_READER_H_

#include "image/result.h"

namespace image {

class ImageInfo;
class ImageFrame;
class ImageMetadata;

class ImageReader {
 public:
  virtual bool HasMoreFrames() const = 0;
  virtual const ImageMetadata* GetMetadata() const = 0;

  // |info| can be null. In this case reader should just try to advance
  // underlying decoder to get the info.
  virtual Result GetImageInfo(ImageInfo** info) = 0;

  // |frame| can be null as well. Same as above.
  virtual Result GetNextFrame(ImageFrame** frame) = 0;

  virtual ~ImageReader() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_READER_H_
