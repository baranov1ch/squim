#ifndef IMAGE_OPTIMIZATION_STRATEGY_H_
#define IMAGE_OPTIMIZATION_STRATEGY_H_

#include "image/result.h"

namespace image {

class OptimizationStrategy {
 public:
  virtual Result ShouldEvenBother() = 0;
  virtual ImageType GetOutputType() = 0;
  virtual Result AdjustImageReader(std::unique_ptr<ImageReader>* reader) = 0;
  virtual Result AdjustImageWriter(std::unique_ptr<ImageWriter>* writer) = 0;
  virtual bool ShouldWaitForMetadata() = 0;

  virtual ~OptimizationStrategy() {}
};

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_STRATEGY_H_
