/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#include "squim/image/optimization/lazy_webp_writer.h"

#include "squim/base/memory/make_unique.h"
#include "squim/image/image_codec_factory.h"
#include "squim/image/image_decoder.h"
#include "squim/image/image_frame.h"
#include "squim/image/image_info.h"
#include "squim/image/image_metadata.h"
#include "squim/image/image_optimization_stats.h"
#include "squim/image/test/mock_encoder.h"
#include "squim/io/writer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::Invoke;
using testing::Return;

namespace image {

namespace {

class MockCodecFactory : public ImageCodecFactory {
 public:
  std::unique_ptr<ImageDecoder> CreateDecoder(
      ImageType type,
      std::unique_ptr<io::BufReader> reader) override {
    return std::unique_ptr<ImageDecoder>(CreateDecoderImpl(type, reader.get()));
  }

  std::unique_ptr<ImageEncoder> CreateEncoder(
      ImageType type,
      std::unique_ptr<io::VectorWriter> writer) override {
    return std::unique_ptr<ImageEncoder>(CreateEncoderImpl(type, writer.get()));
  }

  MOCK_METHOD2(CreateDecoderImpl, ImageDecoder*(ImageType, io::BufReader*));
  MOCK_METHOD2(CreateEncoderImpl, ImageEncoder*(ImageType, io::VectorWriter*));
};

}  // namespace

class LazyWebPWriterTest : public testing::Test {
 public:
  void SetUp() override {
    image_info_.type = ImageType::kJpeg;
    auto dest = base::make_unique<io::DevNull>();
    testee_.reset(
        new LazyWebPWriter(std::move(dest), &codec_factory_, &image_info_));
  }

  void CreateEncoder() {
    DCHECK(!mock_encoder_);
    mock_encoder_ = new MockEncoder();
  }

  MockEncoder* GetEncoder(ImageType type, io::VectorWriter* writer) {
    return mock_encoder_;
  }

 protected:
  MockCodecFactory codec_factory_;
  ImageInfo image_info_;
  std::unique_ptr<LazyWebPWriter> testee_;
  MockEncoder* mock_encoder_ = nullptr;
};

TEST_F(LazyWebPWriterTest, CreateAndWrite) {
  CreateEncoder();
  image_info_.type = ImageType::kGif;
  ImageMetadata metadata;
  ImageFrame frame;
  testee_->SetMetadata(&metadata);
  EXPECT_CALL(codec_factory_, CreateEncoderImpl(ImageType::kWebP, _))
      .WillOnce(Invoke(this, &LazyWebPWriterTest::GetEncoder));
  EXPECT_CALL(*mock_encoder_, Initialize(&image_info_))
      .WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*mock_encoder_, SetMetadata(&metadata));
  EXPECT_CALL(*mock_encoder_, EncodeFrame(&frame, false))
      .Times(2)
      .WillRepeatedly(Return(Result::Ok()));
  auto result = testee_->WriteFrame(&frame);
  EXPECT_TRUE(result.ok());

  result = testee_->WriteFrame(&frame);
  EXPECT_TRUE(result.ok());
}

TEST_F(LazyWebPWriterTest, CreateAndWriteNoMetadata) {
  CreateEncoder();
  ImageFrame frame;
  EXPECT_CALL(codec_factory_, CreateEncoderImpl(ImageType::kWebP, _))
      .WillOnce(Invoke(this, &LazyWebPWriterTest::GetEncoder));
  EXPECT_CALL(*mock_encoder_, Initialize(&image_info_))
      .WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*mock_encoder_, SetMetadata(_)).Times(0);
  EXPECT_CALL(*mock_encoder_, EncodeFrame(&frame, true))
      .WillOnce(Return(Result::Ok()));
  auto result = testee_->WriteFrame(&frame);
  EXPECT_TRUE(result.ok());
}

TEST_F(LazyWebPWriterTest, FinishNoInner) {
  ImageOptimizationStats stats;
  auto result = testee_->FinishWrite(&stats);
  EXPECT_TRUE(result.ok());
}

TEST_F(LazyWebPWriterTest, FinishShouldCallInner) {
  CreateEncoder();
  ImageFrame frame;
  EXPECT_CALL(codec_factory_, CreateEncoderImpl(ImageType::kWebP, _))
      .WillOnce(Invoke(this, &LazyWebPWriterTest::GetEncoder));
  EXPECT_CALL(*mock_encoder_, Initialize(&image_info_))
      .WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*mock_encoder_, SetMetadata(_)).Times(0);
  EXPECT_CALL(*mock_encoder_, EncodeFrame(&frame, true))
      .WillOnce(Return(Result::Ok()));
  auto result = testee_->WriteFrame(&frame);
  EXPECT_TRUE(result.ok());

  ImageOptimizationStats stats;
  EXPECT_CALL(*mock_encoder_, FinishWrite(&stats))
      .WillOnce(Return(Result::Error(Result::Code::kEncodeError)));
  result = testee_->FinishWrite(&stats);
  EXPECT_EQ(Result::Code::kEncodeError, result.code());
}

TEST_F(LazyWebPWriterTest, WriteErrorIfNoEncoder) {
  ImageFrame frame;
  EXPECT_CALL(codec_factory_, CreateEncoderImpl(ImageType::kWebP, _))
      .WillOnce(Return(nullptr));
  auto result = testee_->WriteFrame(&frame);
  EXPECT_EQ(Result::Code::kDunnoHowToEncode, result.code());
}

TEST_F(LazyWebPWriterTest, WriteErrorIfInnerInitError) {
  CreateEncoder();
  ImageFrame frame;
  EXPECT_CALL(codec_factory_, CreateEncoderImpl(ImageType::kWebP, _))
      .WillOnce(Invoke(this, &LazyWebPWriterTest::GetEncoder));
  EXPECT_CALL(*mock_encoder_, Initialize(&image_info_))
      .WillOnce(Return(Result::Error(Result::Code::kDunnoHowToEncode)));
  auto result = testee_->WriteFrame(&frame);
  EXPECT_EQ(Result::Code::kDunnoHowToEncode, result.code());
}

}  // namespace image
