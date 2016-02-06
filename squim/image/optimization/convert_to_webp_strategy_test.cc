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

#include "squim/image/optimization/convert_to_webp_strategy.h"

#include "squim/base/memory/make_unique.h"
#include "squim/image/image_codec_factory.h"
#include "squim/image/image_reader.h"
#include "squim/image/image_writer.h"
#include "squim/image/test/mock_decoder.h"
#include "squim/image/test/mock_encoder.h"
#include "squim/image/test/mock_image_reader.h"
#include "squim/io/buf_reader.h"
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

MockDecoder* CreateDecoder(ImageType type, io::BufReader* reader) {
  return new MockDecoder();
}

MockEncoder* CreateEncoder(ImageType type, io::VectorWriter* writer) {
  return new MockEncoder();
}

}  // namespace

class ConvertToWebPStrategyTest : public testing::Test {
 protected:
  void SetUp() override {
    testee_ = base::make_unique<ConvertToWebPStrategy>(
        [this](CodecConfigurator* configurator)
            -> std::unique_ptr<ImageCodecFactory> {
              auto codec_factory = base::make_unique<MockCodecFactory>();
              codec_factory_ = codec_factory.get();
              return std::move(codec_factory);
            });
  }

  std::unique_ptr<ConvertToWebPStrategy> testee_;
  MockDecoder* decoder_;
  MockCodecFactory* codec_factory_;
};

TEST_F(ConvertToWebPStrategyTest, ShouldAlwaysBother) {
  EXPECT_TRUE(testee_->ShouldEvenBother().ok());
}

TEST_F(ConvertToWebPStrategyTest, ShouldNotWaitForMetadata) {
  EXPECT_FALSE(testee_->ShouldWaitForMetadata());
}

TEST_F(ConvertToWebPStrategyTest, ShouldDoNothingOnAdjustment) {
  std::unique_ptr<ImageReader> reader;
  EXPECT_TRUE(testee_->AdjustImageReaderAfterInfoReady(&reader).ok());
  EXPECT_FALSE(reader);
}

TEST_F(ConvertToWebPStrategyTest, ShouldReturnErrorIfNoDecoder) {
  auto src = io::BufReader::CreateEmpty();
  std::unique_ptr<ImageReader> reader;
  EXPECT_CALL(*codec_factory_, CreateDecoderImpl(ImageType::kJpeg, src.get()))
      .WillOnce(Return(nullptr));
  auto result =
      testee_->CreateImageReader(ImageType::kJpeg, std::move(src), &reader);
  EXPECT_FALSE(reader);
  EXPECT_EQ(Result::Code::kUnsupportedFormat, result.code());
}

TEST_F(ConvertToWebPStrategyTest, ShouldCreateDecoder) {
  auto src = io::BufReader::CreateEmpty();
  std::unique_ptr<ImageReader> reader;
  EXPECT_CALL(*codec_factory_, CreateDecoderImpl(ImageType::kJpeg, src.get()))
      .WillOnce(Invoke(CreateDecoder));
  auto result =
      testee_->CreateImageReader(ImageType::kJpeg, std::move(src), &reader);
  EXPECT_TRUE(reader);
  EXPECT_TRUE(result.ok());
}

TEST_F(ConvertToWebPStrategyTest, ShouldReturnErrorIfNoEncoder) {
  auto dest = base::make_unique<io::DevNull>();
  MockImageReader reader;
  reader.image_info.type = ImageType::kJpeg;
  std::unique_ptr<ImageWriter> writer;
  EXPECT_CALL(reader, GetImageInfo(_))
      .WillOnce(Invoke(&reader, &MockImageReader::GetFakeImageInfo));
  EXPECT_CALL(*codec_factory_, CreateEncoderImpl(ImageType::kWebP, dest.get()))
      .WillOnce(Return(nullptr));
  auto result = testee_->CreateImageWriter(std::move(dest), &reader, &writer);
  EXPECT_FALSE(writer);
  EXPECT_EQ(Result::Code::kDunnoHowToEncode, result.code());
}

TEST_F(ConvertToWebPStrategyTest, ShouldReturnErrorIfWebP) {
  auto dest = base::make_unique<io::DevNull>();
  MockImageReader reader;
  std::unique_ptr<ImageWriter> writer;
  EXPECT_CALL(reader, GetImageInfo(_))
      .WillRepeatedly(Invoke(&reader, &MockImageReader::GetFakeImageInfo));

  reader.image_info.type = ImageType::kWebP;
  auto result = testee_->CreateImageWriter(std::move(dest), &reader, &writer);
  EXPECT_FALSE(writer);
  EXPECT_EQ(Result::Code::kDunnoHowToEncode, result.code());
}

TEST_F(ConvertToWebPStrategyTest, ShouldCreateEncoder) {
  auto dest = base::make_unique<io::DevNull>();
  MockImageReader reader;
  reader.image_info.type = ImageType::kJpeg;
  std::unique_ptr<ImageWriter> writer;
  EXPECT_CALL(reader, GetImageInfo(_))
      .WillOnce(Invoke(&reader, &MockImageReader::GetFakeImageInfo));
  EXPECT_CALL(*codec_factory_, CreateEncoderImpl(ImageType::kWebP, dest.get()))
      .WillOnce(Invoke(CreateEncoder));
  auto result = testee_->CreateImageWriter(std::move(dest), &reader, &writer);
  EXPECT_TRUE(writer);
  EXPECT_TRUE(result.ok());
}

TEST_F(ConvertToWebPStrategyTest, CodecConfigurations) {
  // Only RGB(A) is allowed.

  auto gif_params = testee_->GetGifDecoderParams();
  EXPECT_TRUE(gif_params.color_scheme_allowed(ColorScheme::kRGB));
  EXPECT_TRUE(gif_params.color_scheme_allowed(ColorScheme::kRGBA));
  EXPECT_FALSE(gif_params.color_scheme_allowed(ColorScheme::kGrayScale));
  EXPECT_FALSE(gif_params.color_scheme_allowed(ColorScheme::kGrayScaleAlpha));
  EXPECT_FALSE(gif_params.color_scheme_allowed(ColorScheme::kYUV));
  EXPECT_FALSE(gif_params.color_scheme_allowed(ColorScheme::kYUVA));

  auto jpeg_params = testee_->GetJpegDecoderParams();
  EXPECT_TRUE(jpeg_params.color_scheme_allowed(ColorScheme::kRGB));
  EXPECT_TRUE(jpeg_params.color_scheme_allowed(ColorScheme::kRGBA));
  EXPECT_FALSE(jpeg_params.color_scheme_allowed(ColorScheme::kGrayScale));
  EXPECT_FALSE(jpeg_params.color_scheme_allowed(ColorScheme::kGrayScaleAlpha));
  EXPECT_FALSE(jpeg_params.color_scheme_allowed(ColorScheme::kYUV));
  EXPECT_FALSE(jpeg_params.color_scheme_allowed(ColorScheme::kYUVA));

  auto png_params = testee_->GetPngDecoderParams();
  EXPECT_TRUE(png_params.color_scheme_allowed(ColorScheme::kRGB));
  EXPECT_TRUE(png_params.color_scheme_allowed(ColorScheme::kRGBA));
  EXPECT_FALSE(png_params.color_scheme_allowed(ColorScheme::kGrayScale));
  EXPECT_FALSE(png_params.color_scheme_allowed(ColorScheme::kGrayScaleAlpha));
  EXPECT_FALSE(png_params.color_scheme_allowed(ColorScheme::kYUV));
  EXPECT_FALSE(png_params.color_scheme_allowed(ColorScheme::kYUVA));
}

}  // namespace image
