#ifndef IMAGE_IMAGE_CODEC_FACTORY_H_
#define IMAGE_IMAGE_CODEC_FACTORY_H_

#include <memory>

#include "image/image_constants.h"

namespace io {
class BufReader;
class Writer;
}

namespace image {

class ImageDecoder;
class ImageEncoder;

// Abstract codec factory interface.
class ImageCodecFactory {
 public:
  virtual std::unique_ptr<ImageDecoder> CreateDecoder(
      ImageType type,
      std::unique_ptr<io::BufReader> reader) = 0;
  virtual std::unique_ptr<ImageEncoder> CreateEncoder(
      ImageType type,
      std::unique_ptr<io::Writer> writer) = 0;

  virtual ~ImageCodecFactory() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_CODEC_FACTORY_H_
