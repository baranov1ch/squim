/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IMAGE_OPTIMIZATION_IMAGE_OPTIMIZER_H_
#define IMAGE_OPTIMIZATION_IMAGE_OPTIMIZER_H_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>

#include "base/make_noncopyable.h"
#include "image/image_constants.h"
#include "image/image_writer.h"
#include "image/result.h"

namespace io {
class BufReader;
class VectorWriter;
}

namespace image {

class ImageFrame;
class ImageReader;
class ImageReaderWriterFactory;
class OptimizationStrategy;

class ImageOptimizer {
  MAKE_NONCOPYABLE(ImageOptimizer);

 public:
  static constexpr size_t kLongestSignatureMatch = sizeof("RIFF????WEBPVP") - 1;

  using ImageTypeSelector = std::function<Result(io::BufReader*, ImageType*)>;

  static ImageType ChooseImageType(
      const uint8_t signature[kLongestSignatureMatch]);

  static Result DefaultImageTypeSelector(io::BufReader* reader,
                                         ImageType* image_type);

  ImageOptimizer(ImageTypeSelector input_type_selector,
                 std::unique_ptr<OptimizationStrategy> strategy,
                 std::unique_ptr<io::BufReader> source,
                 std::unique_ptr<io::VectorWriter> dest);
  ~ImageOptimizer();

  Result Process();
  bool Finished() const;
  const ImageWriter::Stats& stats() const { return stats_; }

 private:
  enum class State {
    kInit,
    kReadingFormat,
    kReadingImageInfo,
    kReadFrame,
    kWriteFrame,
    kDrain,
    kFinish,
    kComplete,
    kNone,
  };

  Result DoLoop(Result result);

  Result DoInit();
  Result DoReadImageFormat();
  Result DoReadImageInfo();
  Result DoReadFrame();
  Result DoWriteFrame();
  Result DoDrain();
  Result DoFinish();
  Result DoComplete();

  friend std::ostream& operator<<(std::ostream& os,
                                  ImageOptimizer::State state);
  static const char* StateToString(State state);

  State state_ = State::kInit;

  ImageTypeSelector input_type_selector_;
  std::unique_ptr<OptimizationStrategy> strategy_;
  std::unique_ptr<ImageReader> reader_;
  std::unique_ptr<ImageWriter> writer_;
  std::unique_ptr<io::BufReader> source_;
  std::unique_ptr<io::VectorWriter> dest_;
  ImageFrame* current_frame_ = nullptr;
  Result last_result_ = Result::Ok();
  ImageWriter::Stats stats_;
};

std::ostream& operator<<(std::ostream& os, ImageOptimizer::State state);

}  // namespace image

#endif  // IMAGE_OPTIMIZATION_IMAGE_OPTIMIZER_H_
