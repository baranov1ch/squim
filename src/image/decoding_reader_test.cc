#include "image/decoding_reader.h"

#include <memory>

#include "base/memory/make_unique.h"
#include "image/image_decoder.h"
#include "image/image_frame.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::InSequence;
using testing::Return;

namespace image {

namespace {

class MockDecoder : public ImageDecoder {
 public:
  MOCK_CONST_METHOD0(GetWidth, uint32_t());
  MOCK_CONST_METHOD0(GetHeight, uint32_t());
  MOCK_CONST_METHOD0(GetSize, uint64_t());
  MOCK_CONST_METHOD0(GetImageType, ImageType());
  MOCK_CONST_METHOD0(GetColorScheme, ColorScheme());
  MOCK_CONST_METHOD0(IsProgressive, bool());
  MOCK_CONST_METHOD0(IsImageInfoComplete, bool());
  MOCK_CONST_METHOD0(GetFrameCount, size_t());
  MOCK_CONST_METHOD0(IsMultiFrame, bool());
  MOCK_CONST_METHOD0(GetEstimatedQuality, uint32_t());
  MOCK_CONST_METHOD1(IsFrameCompleteAtIndex, bool(size_t));
  MOCK_METHOD1(GetFrameAtIndex, ImageFrame*(size_t));
  MOCK_METHOD0(GetMetadata, ImageMetadata*());
  MOCK_CONST_METHOD0(IsAllMetadataComplete, bool());
  MOCK_CONST_METHOD0(IsAllFramesComplete, bool());
  MOCK_CONST_METHOD0(IsImageComplete, bool());
  MOCK_METHOD0(Decode, Result());
  MOCK_METHOD0(DecodeImageInfo, Result());
  MOCK_CONST_METHOD0(HasError, bool());
};

}  // namespace

class DecodingReaderTest : public testing::Test {
 protected:
  void SetUp() override {
    auto decoder = base::make_unique<MockDecoder>();
    decoder_ = decoder.get();
    testee_ = base::make_unique<DecodingReader>(std::move(decoder));
  }

  void SetmageInfoExpectations() {
    EXPECT_CALL(*decoder_, IsImageInfoComplete()).WillOnce(Return(false));
    EXPECT_CALL(*decoder_, DecodeImageInfo()).WillOnce(Return(Result::Ok()));
    EXPECT_CALL(*decoder_, GetColorScheme())
        .WillOnce(Return(ColorScheme::kRGBA));
    EXPECT_CALL(*decoder_, GetWidth()).WillOnce(Return(640));
    EXPECT_CALL(*decoder_, GetHeight()).WillOnce(Return(480));
    EXPECT_CALL(*decoder_, GetSize()).WillOnce(Return(6666666));
    EXPECT_CALL(*decoder_, GetImageType()).WillOnce(Return(ImageType::kGif));
    EXPECT_CALL(*decoder_, IsMultiFrame()).WillOnce(Return(true));
    EXPECT_CALL(*decoder_, IsProgressive()).WillOnce(Return(true));
    EXPECT_CALL(*decoder_, GetEstimatedQuality()).WillOnce(Return(75));
  }

  std::unique_ptr<DecodingReader> testee_;
  MockDecoder* decoder_;
};

TEST_F(DecodingReaderTest, ReadingImageInfoPend) {
  EXPECT_CALL(*decoder_, IsImageInfoComplete())
      .Times(2)
      .WillRepeatedly(Return(false));
  EXPECT_CALL(*decoder_, DecodeImageInfo())
      .WillRepeatedly(Return(Result::Pending()));
  auto result = testee_->GetImageInfo(nullptr);
  EXPECT_TRUE(result.pending());
  result = testee_->GetImageInfo(nullptr);
  EXPECT_TRUE(result.pending());
}

TEST_F(DecodingReaderTest, ReadingImageInfoError) {
  EXPECT_CALL(*decoder_, IsImageInfoComplete()).WillOnce(Return(false));
  EXPECT_CALL(*decoder_, DecodeImageInfo())
      .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));
  auto result = testee_->GetImageInfo(nullptr);
  EXPECT_EQ(Result::Code::kDecodeError, result.code());
}

TEST_F(DecodingReaderTest, ReadingImageInfoSuccess) {
  SetmageInfoExpectations();
  const ImageInfo* image_info;
  auto result = testee_->GetImageInfo(&image_info);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(ColorScheme::kRGBA, image_info->color_scheme);
  EXPECT_EQ(640, image_info->width);
  EXPECT_EQ(480, image_info->height);
  EXPECT_EQ(6666666, image_info->size);
  EXPECT_EQ(ImageType::kGif, image_info->type);
  EXPECT_EQ(true, image_info->multiframe);
  EXPECT_EQ(true, image_info->is_progressive);
  EXPECT_EQ(75, image_info->quality);
}

TEST_F(DecodingReaderTest, ReadOneFrame) {
  InSequence seq;
  SetmageInfoExpectations();
  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Pending()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(0)).WillOnce(Return(true));
  ImageFrame frame;
  EXPECT_CALL(*decoder_, GetFrameAtIndex(0)).WillOnce(Return(&frame));
  ImageFrame* out_frame;
  auto result = testee_->GetNextFrame(&out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame, out_frame);
  EXPECT_EQ(1, testee_->GetNumberOfFramesRead());
}

TEST_F(DecodingReaderTest, HasMoreFrames) {
  // Nothing read.
  EXPECT_TRUE(testee_->HasMoreFrames());

  InSequence seq;
  SetmageInfoExpectations();
  auto result = testee_->GetImageInfo(nullptr);
  EXPECT_TRUE(result.ok());

  // After header has been read, look if all frames are decoded.
  EXPECT_CALL(*decoder_, IsAllFramesComplete()).WillOnce(Return(false));
  EXPECT_TRUE(testee_->HasMoreFrames());

  EXPECT_CALL(*decoder_, IsAllFramesComplete()).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameCount()).WillOnce(Return(5));
  EXPECT_TRUE(testee_->HasMoreFrames());
}

TEST_F(DecodingReaderTest, HasNoMoreFrames) {
  InSequence seq;
  SetmageInfoExpectations();
  auto result = testee_->GetImageInfo(nullptr);
  EXPECT_TRUE(result.ok());
  EXPECT_CALL(*decoder_, IsAllFramesComplete()).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameCount()).WillOnce(Return(2));
  EXPECT_TRUE(testee_->HasMoreFrames());

  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(0)).WillOnce(Return(true));

  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(1)).WillOnce(Return(true));

  testee_->GetNextFrame(nullptr);
  testee_->GetNextFrame(nullptr);
  EXPECT_CALL(*decoder_, IsAllFramesComplete()).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameCount()).WillOnce(Return(2));
  EXPECT_FALSE(testee_->HasMoreFrames());
}

TEST_F(DecodingReaderTest, ReadingManyFrames) {
  InSequence seq;
  SetmageInfoExpectations();
  auto result = testee_->GetImageInfo(nullptr);
  EXPECT_TRUE(result.ok());

  ImageFrame frame1;
  ImageFrame frame2;
  ImageFrame frame3;

  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(0)).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameAtIndex(0)).WillOnce(Return(&frame1));

  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(1)).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameAtIndex(1)).WillOnce(Return(&frame2));

  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(2)).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameAtIndex(2)).WillOnce(Return(&frame3));

  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Pending()));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(3)).WillOnce(Return(false));

  ImageFrame* out_frame;
  result = testee_->GetNextFrame(&out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame1, out_frame);
  EXPECT_EQ(1, testee_->GetNumberOfFramesRead());

  result = testee_->GetNextFrame(&out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame2, out_frame);
  EXPECT_EQ(2, testee_->GetNumberOfFramesRead());

  result = testee_->GetNextFrame(&out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame3, out_frame);
  EXPECT_EQ(3, testee_->GetNumberOfFramesRead());

  result = testee_->GetNextFrame(&out_frame);
  EXPECT_TRUE(result.pending());
  EXPECT_EQ(3, testee_->GetNumberOfFramesRead());

  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(0)).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameAtIndex(0)).WillOnce(Return(&frame1));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(1)).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameAtIndex(1)).WillOnce(Return(&frame2));
  EXPECT_CALL(*decoder_, IsFrameCompleteAtIndex(2)).WillOnce(Return(true));
  EXPECT_CALL(*decoder_, GetFrameAtIndex(2)).WillOnce(Return(&frame3));
  result = testee_->GetFrameAtIndex(0, &out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame1, out_frame);

  result = testee_->GetFrameAtIndex(1, &out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame2, out_frame);

  result = testee_->GetFrameAtIndex(2, &out_frame);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(&frame3, out_frame);

  result = testee_->GetFrameAtIndex(3, &out_frame);
  EXPECT_TRUE(result.pending());

  EXPECT_CALL(*decoder_, Decode())
      .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));
  result = testee_->GetNextFrame(&out_frame);
  EXPECT_EQ(Result::Code::kDecodeError, result.code());
}

TEST_F(DecodingReaderTest, ReadTillTheEnd) {
  InSequence seq;
  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(false));
  SetmageInfoExpectations();
  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(false));
  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Pending()));

  auto result = testee_->ReadTillTheEnd();
  EXPECT_TRUE(result.pending());

  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(false));
  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(true));
  result = testee_->ReadTillTheEnd();
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(0, testee_->GetNumberOfFramesRead());
}

TEST_F(DecodingReaderTest, ReadTillTheError) {
  InSequence seq;
  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(false));
  SetmageInfoExpectations();
  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(false));
  EXPECT_CALL(*decoder_, Decode())
      .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));

  auto result = testee_->ReadTillTheEnd();
  EXPECT_EQ(Result::Code::kDecodeError, result.code());
}

}  // namespace image
