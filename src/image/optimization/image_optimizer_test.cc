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

#include "image/optimization/image_optimizer.h"

#include <memory>
#include <vector>

#include "base/memory/make_unique.h"
#include "base/strings/string_util.h"
#include "glog/logging.h"
#include "image/image_frame.h"
#include "image/image_info.h"
#include "image/image_metadata.h"
#include "image/image_reader.h"
#include "image/image_writer.h"
#include "image/optimization/optimization_strategy.h"
#include "io/buf_reader.h"
#include "io/buffered_source.h"
#include "io/writer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::InSequence;
using testing::Invoke;
using testing::Return;

namespace image {

namespace {

class MockStrategy : public OptimizationStrategy {
 public:
  Result CreateImageReader(ImageType image_type,
                           std::unique_ptr<io::BufReader> src,
                           std::unique_ptr<ImageReader>* reader) override {
    return CreateImageReaderImpl(image_type, src.get(), reader);
  }
  Result CreateImageWriter(std::unique_ptr<io::VectorWriter> dest,
                           ImageReader* reader,
                           std::unique_ptr<ImageWriter>* writer) override {
    return CreateImageWriterImpl(dest.get(), reader, writer);
  }
  MOCK_METHOD0(ShouldEvenBother, Result());
  MOCK_METHOD1(AdjustImageReaderAfterInfoReady,
               Result(std::unique_ptr<ImageReader>*));
  MOCK_METHOD3(CreateImageReaderImpl,
               Result(ImageType,
                      io::BufReader*,
                      std::unique_ptr<ImageReader>*));
  MOCK_METHOD3(CreateImageWriterImpl,
               Result(io::VectorWriter*,
                      ImageReader*,
                      std::unique_ptr<ImageWriter>*));
  MOCK_METHOD0(ShouldWaitForMetadata, bool());
};

class MockReader : public ImageReader {
 public:
  MOCK_CONST_METHOD0(HasMoreFrames, bool());
  MOCK_CONST_METHOD0(GetMetadata, const ImageMetadata*());
  MOCK_CONST_METHOD0(GetNumberOfFramesRead, size_t());
  MOCK_METHOD1(GetImageInfo, Result(const ImageInfo**));
  MOCK_METHOD1(GetNextFrame, Result(ImageFrame**));
  MOCK_METHOD2(GetFrameAtIndex, Result(size_t, ImageFrame**));
  MOCK_METHOD0(ReadTillTheEnd, Result());
};

class MockWriter : public ImageWriter {
 public:
  MOCK_METHOD1(SetMetadata, void(const ImageMetadata*));
  MOCK_METHOD1(WriteFrame, Result(ImageFrame*));
  MOCK_METHOD0(FinishWrite, Result());
};

class DevNullWriter : public io::VectorWriter {
 public:
  io::IoResult WriteV(io::ChunkList chunks) override {
    return io::IoResult::Eof();
  }
};

Result TestImageTypeSelector(io::BufReader* reader, ImageType* image_type) {
  const size_t kSignatureLength = 8;
  uint8_t signature[kSignatureLength];
  auto io_result = reader->PeekNInto(signature);
  auto result = Result::FromIoResult(io_result, false);
  if (!result.ok())
    return result;

  EXPECT_EQ(kSignatureLength, io_result.n());
  EXPECT_EQ("testsign", base::StringFromBytes(signature));

  *image_type = ImageType::kJpeg;
  return Result::Ok();
}

std::unique_ptr<io::BufReader> CreateReaderFromData(
    const std::vector<std::string>& data,
    bool end) {
  auto source =
      base::make_unique<io::BufReader>(base::make_unique<io::BufferedSource>());
  for (const auto& chunk : data)
    source->source()->AddChunk(base::make_unique<io::StringChunk>(chunk));
  if (end)
    source->source()->SendEof();
  return std::move(source);
}

}  // namespace

class ImageOptimizerTest : public testing::Test {
 public:
  Result SetFrame(ImageFrame** frame) {
    *frame = &frame_;
    return Result::Ok();
  }

  Result CreateReader(ImageType image_type,
                      io::BufReader* src,
                      std::unique_ptr<ImageReader>* reader) {
    reader->reset(image_reader_);
    return Result::Ok();
  }

  Result CreateWriter(io::VectorWriter* dst,
                      ImageReader* reader,
                      std::unique_ptr<ImageWriter>* writer) {
    writer->reset(image_writer_);
    return Result::Ok();
  }

 protected:
  enum class Stage {
    kInit,
    kReadSignature,
    kCreateReader,
    kReadImageInfo,
    kCreateWriter,
    kReadFrame,
    kWriteFrame,
    kDrain,
    kFinish,
  };

  void RunTestCaseUntil(Stage stage, Result::Code code) {
    auto current_stage = static_cast<int>(Stage::kInit);
    auto int_stage = static_cast<int>(stage);
    ImageMetadata meta;
    InSequence seq;
    do {
      switch (static_cast<Stage>(current_stage)) {
        case Stage::kInit:
          if (current_stage < int_stage) {
            EXPECT_CALL(*strategy_, ShouldEvenBother())
                .WillOnce(Return(Result::Ok()));
          } else {
            EXPECT_CALL(*strategy_, ShouldEvenBother())
                .WillOnce(Return(Result::Finish(code)));
          }
          break;
        case Stage::kReadSignature:
          // Everything is defined by the reader.
          break;
        case Stage::kCreateReader:
          if (current_stage < int_stage) {
            image_reader_ = new MockReader();
            EXPECT_CALL(*strategy_,
                        CreateImageReaderImpl(ImageType::kJpeg, source_, _))
                .WillOnce(Invoke(this, &ImageOptimizerTest::CreateReader));
          } else {
            EXPECT_CALL(*strategy_,
                        CreateImageReaderImpl(ImageType::kJpeg, source_, _))
                .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));
          }
          break;
        case Stage::kReadImageInfo:
          ASSERT_TRUE(image_reader_);
          if (current_stage < int_stage) {
            EXPECT_CALL(*image_reader_, GetImageInfo(nullptr))
                .WillOnce(Return(Result::Ok()));
          } else if (code == Result::Code::kPending) {
            EXPECT_CALL(*image_reader_, GetImageInfo(nullptr))
                .WillOnce(Return(Result::Pending()));
          } else {
            EXPECT_CALL(*image_reader_, GetImageInfo(nullptr))
                .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));
          }
          break;
        case Stage::kCreateWriter:
          if (current_stage < int_stage) {
            image_writer_ = new MockWriter();
            EXPECT_CALL(*strategy_,
                        CreateImageWriterImpl(dest_, image_reader_, _))
                .WillOnce(Invoke(this, &ImageOptimizerTest::CreateWriter));
          } else {
            EXPECT_CALL(*strategy_,
                        CreateImageWriterImpl(dest_, image_reader_, _))
                .WillOnce(
                    Return(Result::Error(Result::Code::kDunnoHowToEncode)));
          }
          break;
        case Stage::kReadFrame:
          EXPECT_CALL(*image_reader_, GetMetadata()).WillOnce(Return(&meta));
          EXPECT_CALL(*image_writer_, SetMetadata(&meta));
          EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(true));
          if (current_stage < int_stage) {
            EXPECT_CALL(*image_reader_, GetNextFrame(_))
                .WillOnce(Invoke(this, &ImageOptimizerTest::SetFrame));
          } else if (code == Result::Code::kPending) {
            EXPECT_CALL(*image_reader_, GetNextFrame(_))
                .WillOnce(Return(Result::Pending()));
          } else {
            EXPECT_CALL(*image_reader_, GetNextFrame(_))
                .WillOnce(Return(Result::Error(Result::Code::kReadFrameError)));
          }
          break;
        case Stage::kWriteFrame:
          ASSERT_TRUE(image_writer_);
          if (current_stage < int_stage) {
            EXPECT_CALL(*image_writer_, WriteFrame(_))
                .WillOnce(Return(Result::Ok()));
          } else if (code == Result::Code::kPending) {
            EXPECT_CALL(*image_writer_, WriteFrame(_))
                .WillOnce(Return(Result::Pending()));
          } else {
            EXPECT_CALL(*image_writer_, WriteFrame(_))
                .WillOnce(
                    Return(Result::Error(Result::Code::kWriteFrameError)));
          }
          break;
        case Stage::kDrain:
          EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(false));

          EXPECT_CALL(*strategy_, ShouldWaitForMetadata())
              .WillOnce(Return(should_wait_meta_));
          if (should_wait_meta_) {
            if (current_stage < int_stage) {
              EXPECT_CALL(*image_reader_, ReadTillTheEnd())
                  .WillOnce(Return(Result::Ok()));
            } else if (code == Result::Code::kPending) {
              EXPECT_CALL(*image_reader_, ReadTillTheEnd())
                  .WillOnce(Return(Result::Pending()));
            } else {
              EXPECT_CALL(*image_reader_, ReadTillTheEnd())
                  .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));
            }
          } else {
            EXPECT_CALL(*image_reader_, ReadTillTheEnd()).Times(0);
          }
          break;
        case Stage::kFinish:
          if (code == Result::Code::kOk) {
            EXPECT_CALL(*image_writer_, FinishWrite())
                .WillOnce(Return(Result::Ok()));
          } else if (code == Result::Code::kPending) {
            EXPECT_CALL(*image_writer_, FinishWrite())
                .WillOnce(Return(Result::Pending()));
          } else {
            EXPECT_CALL(*image_writer_, FinishWrite())
                .WillOnce(Return(Result::Error(Result::Code::kIoErrorOther)));
          }
          break;
        default:
          break;
      }
    } while (++current_stage <= int_stage);
    auto result = testee_->Process();
    EXPECT_EQ(code, result.code());
    if (code == Result::Code::kPending) {
      EXPECT_TRUE(result.pending());
      EXPECT_FALSE(testee_->Finished());
    } else if (code >= Result::Code::kErrorStart) {
      EXPECT_TRUE(result.error());
      EXPECT_TRUE(testee_->Finished());
    } else {
      EXPECT_TRUE(result.finished());
      EXPECT_TRUE(testee_->Finished());
    }
  }

  std::unique_ptr<ImageOptimizer> CreateOptimizer() {
    auto data =
        std::vector<std::string>({std::string("test"), std::string("sign")});
    return CreateOptimizerWithData(data, true);
  }

  std::unique_ptr<ImageOptimizer> CreateOptimizerWithData(
      const std::vector<std::string>& data,
      bool end) {
    return CreateOptimizerWithDataAndSelector(data, end, TestImageTypeSelector);
  }

  std::unique_ptr<ImageOptimizer> CreateOptimizerWithDataAndSelector(
      const std::vector<std::string>& data,
      bool end,
      ImageOptimizer::ImageTypeSelector image_type_selector) {
    auto source = CreateReaderFromData(data, end);
    auto strategy = base::make_unique<MockStrategy>();
    auto dest = base::make_unique<DevNullWriter>();
    strategy_ = strategy.get();
    source_ = source.get();
    dest_ = dest.get();
    return base::make_unique<ImageOptimizer>(
        image_type_selector, std::move(strategy), std::move(source),
        std::move(dest));
  }

  MockStrategy* strategy_ = nullptr;
  io::BufReader* source_ = nullptr;
  DevNullWriter* dest_ = nullptr;
  MockReader* image_reader_ = nullptr;
  MockWriter* image_writer_ = nullptr;
  ImageFrame frame_;
  std::unique_ptr<ImageOptimizer> testee_;
  bool should_wait_meta_ = false;
};

TEST_F(ImageOptimizerTest, ImageTypeChoice) {
  struct {
    const char data[15];
    ImageType expected_type;
  } kCases[] = {
      {"\xFF\xD8\xFF"
       "sflj123jioj",
       ImageType::kJpeg},
      {"\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
       "23jioj",
       ImageType::kPng},
      {"GIF87alj123jio", ImageType::kGif},
      {"GIF89alj123jio", ImageType::kGif},
      {"RIFF?!@?WEBPVP", ImageType::kWebP},
      {"lkjsnw1284jsoi", ImageType::kUnknown},
  };

  for (const auto c : kCases) {
    auto sig = reinterpret_cast<const uint8_t*>(c.data);
    EXPECT_EQ(c.expected_type, ImageOptimizer::ChooseImageType(sig));
  }
}

TEST_F(ImageOptimizerTest, ShouldStopOnInitIfStrategyDecides) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kInit, Result::Code::kImageTooSmall);
}

TEST_F(ImageOptimizerTest, ShouldPendOnIncompleteSignature) {
  auto data = std::vector<std::string>({std::string("test")});
  testee_ = CreateOptimizerWithData(data, false);
  RunTestCaseUntil(Stage::kReadSignature, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfNoImageTypeSelected) {
  testee_ = CreateOptimizerWithDataAndSelector(
      std::vector<std::string>(), false,
      [](io::BufReader* b, ImageType* type) -> Result { return Result::Ok(); });
  RunTestCaseUntil(Stage::kReadSignature, Result::Code::kUnsupportedFormat);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfReaderCreationFailed) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kCreateReader, Result::Code::kDecodeError);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfImageInfoError) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kReadImageInfo, Result::Code::kDecodeError);
}

TEST_F(ImageOptimizerTest, ShouldPendOnImageInfo) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kReadImageInfo, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfWriterCreationFailed) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kCreateWriter, Result::Code::kDunnoHowToEncode);
}

TEST_F(ImageOptimizerTest, ShouldSetMetadataAndPendOnReadingFrame) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kReadFrame, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfReadFrameReturnsError) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kReadFrame, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldPendOnWritingFrame) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kWriteFrame, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfWriteFrameReturnsError) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kWriteFrame, Result::Code::kWriteFrameError);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfDrainReturnsError) {
  testee_ = CreateOptimizer();
  should_wait_meta_ = true;
  RunTestCaseUntil(Stage::kDrain, Result::Code::kDecodeError);
}

TEST_F(ImageOptimizerTest, ShouldPendOnDrain) {
  testee_ = CreateOptimizer();
  should_wait_meta_ = true;
  RunTestCaseUntil(Stage::kDrain, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldPendOnFinishAfterDrain) {
  testee_ = CreateOptimizer();
  should_wait_meta_ = true;
  RunTestCaseUntil(Stage::kFinish, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldPendOnFinishing) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kFinish, Result::Code::kPending);
}

TEST_F(ImageOptimizerTest, ShouldReturnErrorIfFinishWriteReturnsError) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kFinish, Result::Code::kIoErrorOther);
}

TEST_F(ImageOptimizerTest, ShouldFinish) {
  testee_ = CreateOptimizer();
  RunTestCaseUntil(Stage::kFinish, Result::Code::kOk);
}

TEST_F(ImageOptimizerTest, ShouldTolerateMultilePendingCalls) {
  testee_ = CreateOptimizer();
  InSequence seq;
  EXPECT_CALL(*strategy_, ShouldEvenBother()).WillOnce(Return(Result::Ok()));
  image_reader_ = new MockReader();
  EXPECT_CALL(*strategy_, CreateImageReaderImpl(ImageType::kJpeg, source_, _))
      .WillOnce(Invoke(this, &ImageOptimizerTest::CreateReader));
  EXPECT_CALL(*image_reader_, GetImageInfo(nullptr))
      .WillRepeatedly(Return(Result::Pending()));
  auto result = testee_->Process();
  EXPECT_TRUE(result.pending());
  result = testee_->Process();
  EXPECT_TRUE(result.pending());
  EXPECT_CALL(*image_reader_, GetImageInfo(nullptr))
      .WillOnce(Return(Result::Ok()));
  image_writer_ = new MockWriter();
  EXPECT_CALL(*strategy_, CreateImageWriterImpl(dest_, image_reader_, _))
      .WillOnce(Invoke(this, &ImageOptimizerTest::CreateWriter));
  ImageMetadata meta;
  EXPECT_CALL(*image_reader_, GetMetadata()).WillOnce(Return(&meta));
  EXPECT_CALL(*image_writer_, SetMetadata(&meta));
  EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(true));
  EXPECT_CALL(*image_reader_, GetNextFrame(_))
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());
  EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(true));
  EXPECT_CALL(*image_reader_, GetNextFrame(_))
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());

  EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(true));
  EXPECT_CALL(*image_reader_, GetNextFrame(_))
      .WillOnce(Invoke(this, &ImageOptimizerTest::SetFrame));
  EXPECT_CALL(*image_writer_, WriteFrame(_))
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());

  EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(true));
  EXPECT_CALL(*image_reader_, GetNextFrame(_))
      .WillOnce(Invoke(this, &ImageOptimizerTest::SetFrame));
  EXPECT_CALL(*image_writer_, WriteFrame(_))
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());

  EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(true));
  EXPECT_CALL(*image_reader_, GetNextFrame(_))
      .WillOnce(Invoke(this, &ImageOptimizerTest::SetFrame));
  EXPECT_CALL(*image_writer_, WriteFrame(_)).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*image_reader_, HasMoreFrames()).WillOnce(Return(false));
  EXPECT_CALL(*strategy_, ShouldWaitForMetadata()).WillOnce(Return(true));
  EXPECT_CALL(*image_reader_, ReadTillTheEnd())
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());
  EXPECT_CALL(*image_reader_, ReadTillTheEnd())
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());

  EXPECT_CALL(*image_reader_, ReadTillTheEnd()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*image_writer_, FinishWrite())
      .WillOnce(Return(Result::Pending()));
  result = testee_->Process();
  EXPECT_TRUE(result.pending());

  result = testee_->Process();
  EXPECT_TRUE(result.finished());
}

}  // namespace image
