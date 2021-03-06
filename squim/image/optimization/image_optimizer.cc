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

#include "squim/image/optimization/image_optimizer.h"

#include <cstring>

#include "squim/base/logging.h"
#include "squim/image/image_reader.h"
#include "squim/image/image_writer.h"
#include "squim/image/optimization/optimization_strategy.h"
#include "squim/io/buf_reader.h"
#include "squim/io/writer.h"

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

ImageOptimizer::ImageOptimizer(ImageTypeSelector input_type_selector,
                               std::unique_ptr<OptimizationStrategy> strategy,
                               std::unique_ptr<io::BufReader> source,
                               std::unique_ptr<io::VectorWriter> dest)
    : input_type_selector_(input_type_selector),
      strategy_(std::move(strategy)),
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

  if (state_ == State::kNone)
    return last_result_;

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

    if (result.error() || result.finished()) {
      last_result_ = result;
      state_ = State::kNone;
    }
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

  result =
      strategy_->CreateImageReader(image_type, std::move(source_), &reader_);
  DCHECK(!result.pending());
  if (!result.ok())
    return result;

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

  const ImageInfo* image_info;
  auto result = reader_->GetImageInfo(&image_info);
  if (!result.ok())
    return result;

  result =
      strategy_->CreateImageWriter(std::move(dest_), reader_.get(), &writer_);
  DCHECK(!result.pending());
  if (!result.ok())
    return result;

  result = writer_->Initialize(image_info);
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
  return writer_->FinishWrite(&stats_);
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
