#include "image/image_optimizer.h"

#include <cstring>

#include "glog/logging.h"
#include "image/image_reader.h"
#include "image/image_reader_writer_factory.h"
#include "image/image_writer.h"
#include "image/optimization_strategy.h"
#include "io/buf_reader.h"
#include "io/writer.h"

namespace image {

static_assert(ImageOptimizer::kLongestSignatureMatch == 14,
              "longest signature must be 14 byte long");

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

// static
Result ImageOptimizer::DefaultImageTypeSelector(io::BufReader* reader,
                                                ImageType* image_type) {
  uint8_t signature[kLongestSignatureMatch];
  auto io_result = reader->PeekNInto(signature);
  auto result = Result::FromIoResult(io_result, false);
  if (!result.ok())
    return result;

  CHECK_EQ(kLongestSignatureMatch, io_result.nread());
  *image_type = ChooseImageType(signature);
  return Result::Ok();
}

// static
const char* ImageOptimizer::StateToString(State state) {
  switch (state) {
    case State::kInit:
      return "Init";
    case State::kReadingFormat:
      return "ReadingFormat";
    case State::kReadingImageInfo:
      return "ReadingImageInfo";
    case State::kOptimizing:
      return "Optimizing";
    case State::kNone:
      return "None";
    default:
      return "<unknown>";
  }
}

ImageOptimizer::ImageOptimizer(
    ImageTypeSelector input_type_selector,
    std::unique_ptr<OptimizationStrategy> strategy,
    std::unique_ptr<ImageReaderWriterFactory> factory,
    std::unique_ptr<io::BufReader> source,
    std::unique_ptr<io::Writer> dest)
    : input_type_selector_(input_type_selector),
      strategy_(std::move(strategy)),
      factory_(std::move(factory)),
      source_(std::move(source)),
      dest_(std::move(dest)) {}

ImageOptimizer::~ImageOptimizer() {}

Result ImageOptimizer::Process() {
  return DoLoop(Result::Ok());
}

bool ImageOptimizer::Finished() const {
  return state_ == State::kNone;
}

Result ImageOptimizer::DoLoop(Result result) {
  if (!result.ok())
    return result;

  while (result.ok() && state_ != State::kNone) {
    switch (state_) {
      case State::kInit:
        CHECK(result.ok());
        result = DoInit();
        break;
      case State::kReadingFormat:
        CHECK(result.ok());
        result = DoReadImageFormat();
        break;
      case State::kReadingImageInfo:
        CHECK(result.ok());
        result = DoReadImageInfo();
        break;
      case State::kOptimizing:
        CHECK(result.ok());
        result = DoOptimize();
        break;
      default:
        CHECK(false);
        break;
    };

    if (result.error() || result.finished())
      state_ = State::kNone;
  }

  return result;
}

Result ImageOptimizer::DoInit() {
  CHECK_EQ(State::kInit, state_);
  CHECK(strategy_);
  auto result = strategy_->ShouldEvenBother();
  DCHECK(!result.pending());
  if (!result.ok()) {
    state_ = State::kNone;
  } else {
    state_ = State::kReadingFormat;
  }
  return result;
}

Result ImageOptimizer::DoReadImageFormat() {
  CHECK_EQ(State::kReadingFormat, state_);
  CHECK(source_);
  CHECK(!reader_);
  ImageType image_type = ImageType::kUnknown;
  auto result = input_type_selector_(source_.get(), &image_type);
  if (!result.ok())
    return result;

  if (image_type == ImageType::kUnknown) {
    state_ = State::kNone;
    return Result::Error(Result::Code::kUnsupportedFormat);
  }

  reader_ = factory_->CreateReader(image_type, std::move(source_));
  if (!reader_)
    return Result::Error(Result::Code::kUnsupportedFormat);

  state_ = State::kReadingImageInfo;
  return Result::Ok();
}

Result ImageOptimizer::DoReadImageInfo() {
  CHECK_EQ(State::kReadingImageInfo, state_);
  CHECK(reader_);
  CHECK(!writer_);
  // Source has been transferred.
  CHECK(!source_);
  CHECK(dest_);

  auto result = reader_->GetImageInfo(nullptr);
  if (!result.ok())
    return result;
  result = strategy_->AdjustImageReader(&reader_);
  DCHECK(!result.pending());
  if (!result.ok())
    return result;

  auto output_type = strategy_->GetOutputType();
  if (output_type == ImageType::kUnknown)
    return Result::Error(Result::Code::kDunnoHowToEncode);

  writer_ = factory_->CreateWriterForImage(output_type, reader_.get(),
                                           std::move(dest_));
  if (!writer_)
    return Result::Error(Result::Code::kDunnoHowToEncode);

  result = strategy_->AdjustImageWriter(&writer_);
  DCHECK(!result.pending());
  if (!result.ok())
    return result;

  writer_->SetMetadata(reader_->GetMetadata());

  state_ = State::kOptimizing;
  return Result::Ok();
}

Result ImageOptimizer::DoOptimize() {
  CHECK_EQ(State::kOptimizing, state_);
  CHECK(reader_);
  CHECK(writer_);
  // Both sources must be transferred to coders.
  CHECK(!source_);
  CHECK(!dest_);

  while (reader_->HasMoreFrames()) {
    ImageFrame* frame;
    auto result = reader_->GetNextFrame(&frame);
    if (!result.ok())
      return result;

    CHECK(frame);
    result = writer_->WriteFrame(frame);
    if (!result.ok())
      return result;
  }

  auto result = writer_->FinishWrite();
  if (!result.ok())
    return result;

  return Result::Finish(Result::Code::kOk);
}

std::ostream& operator<<(std::ostream& os, ImageOptimizer::State state) {
  os << ImageOptimizer::StateToString(state);
  return os;
}

}  // namespace image
