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

  CHECK_EQ(kLongestSignatureMatch, io_result.n());
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
    case State::kReadFrame:
      return "ReadFrame";
    case State::kWriteFrame:
      return "WriteFrame";
    case State::kDrain:
      return "Drain";
    case State::kFinish:
      return "Finish";
    case State::kComplete:
      return "Complete";
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
      case State::kReadFrame:
        CHECK(result.ok());
        result = DoReadFrame();
        break;
      case State::kWriteFrame:
        CHECK(result.ok());
        result = DoWriteFrame();
        break;
      case State::kDrain:
        CHECK(result.ok());
        result = DoDrain();
        break;
      case State::kFinish:
        CHECK(result.ok());
        result = DoFinish();
        break;
      case State::kComplete:
        CHECK(result.ok());
        result = DoComplete();
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

  state_ = State::kReadFrame;
  return Result::Ok();
}

Result ImageOptimizer::DoReadFrame() {
  CHECK_EQ(State::kReadFrame, state_);
  CHECK(reader_);
  CHECK(writer_);
  // Both sources must be transferred to coders.
  CHECK(!source_);
  CHECK(!dest_);

  if (!reader_->HasMoreFrames()) {
    if (!strategy_->ShouldWaitForMetadata()) {
      state_ = State::kFinish;
    } else {
      state_ = State::kDrain;
    }
    return Result::Ok();
  }

  current_frame_ = nullptr;
  auto result = reader_->GetNextFrame(&current_frame_);
  if (result.ok()) {
    state_ = State::kWriteFrame;
  }
  return result;
}

Result ImageOptimizer::DoWriteFrame() {
  CHECK_EQ(State::kWriteFrame, state_);

  CHECK(current_frame_);
  state_ = State::kReadFrame;
  return writer_->WriteFrame(current_frame_);
}

Result ImageOptimizer::DoDrain() {
  CHECK_EQ(State::kDrain, state_);
  auto result = reader_->ReadTillTheEnd();
  if (result.ok())
    state_ = State::kFinish;

  return result;
}

Result ImageOptimizer::DoFinish() {
  CHECK_EQ(State::kFinish, state_);
  state_ = State::kComplete;
  return writer_->FinishWrite();
}

Result ImageOptimizer::DoComplete() {
  CHECK_EQ(State::kComplete, state_);
  return Result::Finish(Result::Code::kOk);
}

std::ostream& operator<<(std::ostream& os, ImageOptimizer::State state) {
  os << ImageOptimizer::StateToString(state);
  return os;
}

}  // namespace image
