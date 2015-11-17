#ifndef IMAGE_OPTIMIZATION_STRATEGY_H_
#define IMAGE_OPTIMIZATION_STRATEGY_H_

namespace image {

class OptimizationStrategy {
 public:
  virtual bool ShouldEvenBother() = 0;

  virtual ~OptimizationStrategy() {}
};

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_STRATEGY_H_
