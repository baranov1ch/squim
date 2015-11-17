#include "image/image_optimizer.h"

#include <cstring>

#include "glog/logging.h"
#include "image/image_decoder.h"
#include "image/image_encoder.h"
#include "image/optimization_strategy.h"
#include "io/buf_reader.h"
#include "io/writer.h"

namespace image {

namespace {

bool MatchesJPEGSignature(const uint8_t* contents) {
  return !std::memcmp(contents, "\xFF\xD8\xFF", 3);
}

bool MatchesPNGSignature(const uint8_t* contents) {
  return !std::memcmp(contents, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
}

bool MatchesGIFSignature(const uint8_t* contents) {
  return !std::memcmp(contents, "GIF87a", 6) || !memcmp(contents, "GIF89a", 6);
}

bool MatchesWebPSignature(const uint8_t* contents) {
  return !std::memcmp(contents, "RIFF", 4) &&
         !memcmp(contents + 8, "WEBPVP", 6);
}

}  // namespace

// static
ImageType ImageOptimizer::ChooseImageType(
    const uint8_t signature[kLongestSignatureMatch]) {
  static_assert(kLongestSignatureMatch == 14,
                "longest signature must be 14 byte long");

  if (MatchesJPEGSignature(signature))
    return ImageType::kJpeg;

  if (MatchesPNGSignature(signature))
    return ImageType::kPng;

  if (MatchesGIFSignature(signature))
    return ImageType::kGif;

  if (MatchesWebPSignature(signature))
    return ImageType::kWebP;

  return ImageType::kUnknown;
}

ImageOptimizer::ImageOptimizer(std::unique_ptr<OptimizationStrategy> strategy,
                               std::unique_ptr<io::BufReader> source,
                               std::unique_ptr<io::Writer> dest)
    : strategy_(std::move(strategy)),
      source_(std::move(source)),
      dest_(std::move(dest)) {}

ImageOptimizer::~ImageOptimizer() {}

Result ImageOptimizer::Init() {
  CHECK_EQ(State::kInit, state_);
  CHECK(strategy_);
  auto result = strategy_->ShouldEvenBother();
  if (!result) {
    state_ = State::kNone;
    return Result::Ok();
    // return Result::Finish(result.code(), result.message());
  }

  state_ = State::kReadingFormat;
  return Result::Ok();
}

Result ImageOptimizer::ReadImageFormat() {
  CHECK_EQ(State::kReadingFormat, state_);
  CHECK(source_);
  uint8_t signature[kLongestSignatureMatch];
  auto io_result = source_->PeekNInto(signature, kLongestSignatureMatch);
  if (io_result.pending())
    return Result::Ok();
  // return Result::Pending();

  if (io_result.error() || io_result.eof()) {
    state_ = State::kNone;
    return Result::Ok();
    // return Result::IOError(io_result.eof());
  }

  CHECK_EQ(kLongestSignatureMatch, io_result.nread());
  auto image_type = ChooseImageType(signature);
  if (image_type == ImageType::kUnknown) {
    state_ = State::kNone;
    return Result::Ok();
    // return Result::Error(kUnsupportedFormat, std::string());
  }
  return Result::Ok();
}

}  // namespace image
