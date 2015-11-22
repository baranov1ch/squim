#ifndef IMAGE_IMAGE_OPTIMIZER_H_
#define IMAGE_IMAGE_OPTIMIZER_H_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>

#include "base/make_noncopyable.h"
#include "image/image_constants.h"
#include "image/result.h"

namespace io {
class BufReader;
class Writer;
}

namespace image {

class ImageReader;
class ImageWriter;
class ImageReaderWriterFactory;
class OptimizationStrategy;

class ImageOptimizer {
  MAKE_NONCOPYABLE(ImageOptimizer);

 public:
  static constexpr size_t kLongestSignatureMatch = sizeof("RIFF????WEBPVP") - 1;

  enum class State {
    kInit,
    kReadingFormat,
    kReadingImageInfo,
    kOptimizing,
    kNone,
  };

  using ImageTypeSelector = std::function<Result(io::BufReader*, ImageType*)>;

  static ImageType ChooseImageType(
      const uint8_t signature[kLongestSignatureMatch]);

  static Result DefaultImageTypeSelector(io::BufReader* reader,
                                         ImageType* image_type);

  ImageOptimizer(ImageTypeSelector input_type_selector,
                 std::unique_ptr<OptimizationStrategy> strategy,
                 std::unique_ptr<ImageReaderWriterFactory> factory,
                 std::unique_ptr<io::BufReader> source,
                 std::unique_ptr<io::Writer> dest);
  ~ImageOptimizer();

  Result Process();
  bool Finished() const;

 private:
  Result DoLoop(Result result);

  Result DoInit();
  Result DoReadImageFormat();
  Result DoReadImageInfo();
  Result DoOptimize();

  friend std::ostream& operator<<(std::ostream& os,
                                  ImageOptimizer::State state);
  static const char* StateToString(State state);

  State state_ = State::kInit;

  ImageTypeSelector input_type_selector_;
  std::unique_ptr<OptimizationStrategy> strategy_;
  std::unique_ptr<ImageReaderWriterFactory> factory_;
  std::unique_ptr<ImageReader> reader_;
  std::unique_ptr<ImageWriter> writer_;
  std::unique_ptr<io::BufReader> source_;
  std::unique_ptr<io::Writer> dest_;
};

std::ostream& operator<<(std::ostream& os, ImageOptimizer::State state);

}  // namespace image

#endif  // IMAGE_IMAGE_OPTIMIZER_H_
