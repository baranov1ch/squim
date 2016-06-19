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

#include "squim/io/buf_writer.h"

#include <vector>

#include "squim/base/memory/make_unique.h"
#include "squim/io/chunk.h"
#include "squim/io/writer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::_;
using testing::InSequence;
using testing::Return;

namespace io {

namespace {

MATCHER_P(ChunkEq, variable, "kekeke") {
  return arg->ToString() == std::string(variable);
}

class RecordingWriter : public Writer {
 public:
  IoResult Write(Chunk* chunk) override {
    chunks_.push_back(chunk->Clone());
    return IoResult::Write(chunk->size());
  }

  const std::vector<ChunkPtr>& chunks() const { return chunks_; }

  Chunk* chunk_at(size_t idx) { return (chunks_.begin() + idx)->get(); }

 private:
  std::vector<ChunkPtr> chunks_;
};

class MockWriter : public Writer {
 public:
  MOCK_METHOD1(Write, IoResult(Chunk*));
};

}  // namespace

class BufWriterTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto recorder = base::make_unique<RecordingWriter>();
    recorder_ = recorder.get();
    testee_ = base::make_unique<BufWriter>(20, std::move(recorder));
  }

 protected:
  std::unique_ptr<BufWriter> testee_;
  RecordingWriter* recorder_;
};

TEST_F(BufWriterTest, WriteBuffered) {
  auto data = Chunk::FromString("test");
  auto res = testee_->Write(data.get());
  EXPECT_TRUE(res.ok());
  EXPECT_EQ(4, res.n());
  EXPECT_EQ(4, testee_->buffered());
  EXPECT_EQ(16, testee_->available());
}

TEST_F(BufWriterTest, FlushRegression) {
  auto data = Chunk::FromString("testme");

  for (auto i = 0; i < 4; ++i) {
    auto res = testee_->Write(data.get());
    EXPECT_TRUE(res.ok());
    EXPECT_EQ(6, res.n());
  }

  EXPECT_EQ(4, testee_->buffered());
  EXPECT_EQ(16, testee_->available());

  EXPECT_TRUE(testee_->Flush().ok());
  EXPECT_EQ(0, testee_->buffered());
  EXPECT_EQ(20, testee_->available());

  const auto& chunks = recorder_->chunks();
  EXPECT_EQ(2, chunks.size());
  auto* chunk1 = recorder_->chunk_at(0);
  EXPECT_EQ(20, chunk1->size());
  EXPECT_EQ("testmetestmetestmete", chunk1->ToString());
  auto chunk2 = recorder_->chunk_at(1);
  EXPECT_EQ(4, chunk2->size());
  EXPECT_EQ("stme", chunk2->ToString());
}

TEST(BufWriterTestWithMock, PendingUnderlying) {
  auto owned_mock = base::make_unique<MockWriter>();
  auto* mock = owned_mock.get();
  auto testee = base::make_unique<BufWriter>(20, std::move(owned_mock));

  InSequence seq;
  auto data = Chunk::FromString("this_is_first_20_symthis_is_rest");
  EXPECT_CALL(*mock, Write(ChunkEq("this_is_first_20_sym")))
      .WillOnce(Return(IoResult::Write(10)));
  EXPECT_CALL(*mock, Write(ChunkEq("rst_20_sym")))
      .WillOnce(Return(IoResult::Pending()));
  auto res = testee->Write(data.get());
  EXPECT_EQ(20, res.n());
  EXPECT_TRUE(testee->flushing());
  EXPECT_EQ(0, testee->available());
  EXPECT_EQ(20, testee->buffered());

  auto slice = data->Slice(20);
  res = testee->Write(slice.get());
  EXPECT_TRUE(res.pending());
  EXPECT_TRUE(testee->flushing());
  EXPECT_EQ(0, testee->available());
  EXPECT_EQ(20, testee->buffered());

  EXPECT_CALL(*mock, Write(ChunkEq("rst_20_sym")))
      .WillOnce(Return(IoResult::Write(10)));
  EXPECT_TRUE(testee->Flush().ok());
  EXPECT_FALSE(testee->flushing());
  EXPECT_EQ(20, testee->available());
  EXPECT_EQ(0, testee->buffered());
}

}  // namespace io
