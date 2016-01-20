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

#include "image/decoding_reader.h"

#include "base/memory/make_unique.h"
#include "image/image_decoder.h"
#include "image/image_frame.h"
#include "test/mock_decoder.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::InSequence;
using testing::Return;
using testing::ReturnRef;

namespace image {

class DecodingReaderTest : public testing::Test {
 protected:
  void SetUp() override {
    auto decoder = base::make_unique<MockDecoder>();
    decoder_ = decoder.get();
    testee_ = base::make_unique<DecodingReader>(std::move(decoder));
    image_info_.width = 640;
    image_info_.height = 480;
    image_info_.size = 6666666;
    image_info_.type = ImageType::kGif;
    image_info_.multiframe = true;
  }

  void SetImageInfoExpectations() {
    EXPECT_CALL(*decoder_, IsImageInfoComplete()).WillOnce(Return(false));
    EXPECT_CALL(*decoder_, DecodeImageInfo()).WillOnce(Return(Result::Ok()));
    EXPECT_CALL(*decoder_, GetImageInfo()).WillOnce(ReturnRef(image_info_));
  }

  std::unique_ptr<DecodingReader> testee_;
  MockDecoder* decoder_;
  ImageInfo image_info_;
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
  SetImageInfoExpectations();
  const ImageInfo* image_info;
  auto result = testee_->GetImageInfo(&image_info);
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(640, image_info->width);
  EXPECT_EQ(480, image_info->height);
  EXPECT_EQ(6666666, image_info->size);
  EXPECT_EQ(ImageType::kGif, image_info->type);
  EXPECT_EQ(true, image_info->multiframe);
}

TEST_F(DecodingReaderTest, ReadOneFrame) {
  InSequence seq;
  SetImageInfoExpectations();
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
  SetImageInfoExpectations();
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
  SetImageInfoExpectations();
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
  SetImageInfoExpectations();
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
  SetImageInfoExpectations();
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
  SetImageInfoExpectations();
  EXPECT_CALL(*decoder_, Decode()).WillOnce(Return(Result::Ok()));
  EXPECT_CALL(*decoder_, IsImageComplete()).WillOnce(Return(false));
  EXPECT_CALL(*decoder_, Decode())
      .WillOnce(Return(Result::Error(Result::Code::kDecodeError)));

  auto result = testee_->ReadTillTheEnd();
  EXPECT_EQ(Result::Code::kDecodeError, result.code());
}

}  // namespace image
