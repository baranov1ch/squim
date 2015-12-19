#ifndef IMAGE_OPTIMIZATION_STRATEGY_H_
#define IMAGE_OPTIMIZATION_STRATEGY_H_

#include <memory>

#include "image/result.h"

namespace io {
class BufReader;
class VectorWriter;
}

namespace image {

class ImageReader;
class ImageWriter;

class OptimizationStrategy {
 public:
  virtual Result ShouldEvenBother() = 0;
  virtual Result CreateImageReader(ImageType image_type,
                                   std::unique_ptr<io::BufReader> src,
                                   std::unique_ptr<ImageReader>* reader) = 0;
  virtual Result CreateImageWriter(std::unique_ptr<io::VectorWriter> dest,
                                   ImageReader* reader,
                                   std::unique_ptr<ImageWriter>* writer) = 0;
  virtual Result AdjustImageReaderAfterInfoReady(
      std::unique_ptr<ImageReader>* reader) = 0;
  virtual bool ShouldWaitForMetadata() = 0;

  virtual ~OptimizationStrategy() {}
};

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_STRATEGY_H_
