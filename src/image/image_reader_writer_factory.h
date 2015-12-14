#ifndef IMAGE_IMAGE_READER_WRITER_FACTORY_H_
#define IMAGE_IMAGE_READER_WRITER_FACTORY_H_

#include <memory>

#include "image/image_constants.h"

namespace io {
class BufReader;
class VectorWriter;
}

namespace image {

class ImageReader;
class ImageWriter;

class ImageReaderWriterFactory {
 public:
  virtual std::unique_ptr<ImageReader> CreateReader(
      ImageType type,
      std::unique_ptr<io::BufReader> reader) = 0;
  virtual std::unique_ptr<ImageWriter> CreateWriterForImage(
      ImageType type,
      ImageReader* reader,
      std::unique_ptr<io::VectorWriter> writer) = 0;

  virtual ~ImageReaderWriterFactory() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_READER_WRITER_FACTORY_H_
