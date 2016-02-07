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

#include "squim/image/single_frame_writer.h"

#include "squim/base/memory/make_unique.h"
#include "squim/image/image_frame.h"
#include "squim/image/image_metadata.h"
#include "squim/image/image_optimization_stats.h"
#include "squim/image/image_writer.h"
#include "test/mock_encoder.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Return;

namespace image {

class SingleFrameWriterTest : public testing::Test {
 protected:
  void SetUp() override {
    auto encoder = base::make_unique<MockEncoder>();
    encoder_ = encoder.get();
    testee_ = base::make_unique<SingleFrameWriter>(std::move(encoder));
  }

  std::unique_ptr<SingleFrameWriter> testee_;
  MockEncoder* encoder_;
};

TEST_F(SingleFrameWriterTest, ShouldAskEncoderToSetMetadata) {
  ImageMetadata metadata;
  EXPECT_CALL(*encoder_, SetMetadata(&metadata));
  testee_->SetMetadata(&metadata);
}

TEST_F(SingleFrameWriterTest, ShouldAskEncoderToFinish) {
  ImageOptimizationStats stats;
  EXPECT_CALL(*encoder_, FinishWrite(&stats)).WillOnce(Return(Result::Ok()));
  auto result = testee_->FinishWrite(&stats);
  EXPECT_TRUE(result.ok());
}

TEST_F(SingleFrameWriterTest, ShouldToEncodeAsLastFrame) {
  ImageFrame frame;
  EXPECT_CALL(*encoder_, EncodeFrame(&frame, true))
      .WillOnce(Return(Result::Ok()));
  auto result = testee_->WriteFrame(&frame);
  EXPECT_TRUE(result.ok());

  result = testee_->WriteFrame(&frame);
  EXPECT_EQ(Result::Code::kFailed, result.code());
}

}  // namespace image
