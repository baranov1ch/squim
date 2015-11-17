#ifndef IMAGE_IMAGE_OPTIMIZER_H_
#define IMAGE_IMAGE_OPTIMIZER_H_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>

#include "image/image_constants.h"
#include "image/result.h"

namespace io {
class BufReader;
class Writer;
}

namespace image {

class ImageDecoder;
class ImageEncoder;
class OptimizationStrategy;

class ImageOptimizer {
 public:
  static constexpr size_t kLongestSignatureMatch = sizeof("RIFF????WEBPVP") - 1;

  enum class State {
    kInit,
    kReadingFormat,
    kReadingImageInfo,
    kOptimizing,
    kNone,
  };

  static ImageType ChooseImageType(
      const uint8_t signature[kLongestSignatureMatch]);

  ImageOptimizer(std::unique_ptr<OptimizationStrategy> strategy,
                 std::unique_ptr<io::BufReader> source,
                 std::unique_ptr<io::Writer> dest);
  ~ImageOptimizer();

  Result Init();
  Result ReadImageFormat();
  Result ReadImageInfo();
  Result Optimize();

  State state() const { return state_; }

 private:
  friend std::ostream& operator<<(std::ostream& os,
                                  ImageOptimizer::State state);
  static const char* StateToString(State state);

  State state_ = State::kNone;

  std::unique_ptr<OptimizationStrategy> strategy_;
  std::unique_ptr<ImageDecoder> input_;
  std::unique_ptr<ImageEncoder> output_;
  std::unique_ptr<io::BufReader> source_;
  std::unique_ptr<io::Writer> dest_;
};

std::ostream& operator<<(std::ostream& os, ImageOptimizer::State state);

}  // namespace image

#endif  // IMAGE_IMAGE_OPTIMIZER_H_
