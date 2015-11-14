#ifndef IMAGE_IMAGE_ENCODER_H_
#define IMAGE_IMAGE_ENCODER_H_

#include <cstddef>
#include <cstdint>
#include <memory>

#include "image/image_constants.h"

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

  static ImageType ChooseImageType(const uint8_t signature[kLongestSignatureMatch]);

  ImageOptimizer(std::unique_ptr<OptimizationStrategy> strategy,
                 std::unique_ptr<io::BufReader> source,
                 std::unique_ptr<io::Writer> dest);
  ~ImageOptimizer();

  Result Init();

  Result Process();

 private:
  enum class State {
    kReadingSignature,
    kReadingImageInfo,
    kOptimizing,
    kNone,
  };

  State state_ = State::kNone;

  std::unique_ptr<OptimizationStrategy> strategy_;
  std::unique_ptr<ImageDecoder> input_;
  std::unique_ptr<ImageEncoder> output_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_ENCODER_H_
