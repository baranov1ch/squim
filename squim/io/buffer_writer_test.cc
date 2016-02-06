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

#include "squim/io/buffer_writer.h"

#include "squim/base/memory/make_unique.h"

#include "gtest/gtest.h"

namespace io {

class BufferWriterTest : public ::testing::Test {
 public:
  void SetUp() override { testee_ = base::make_unique<BufferWriter>(5); }

 protected:
  std::unique_ptr<BufferWriter> testee_;
};

TEST_F(BufferWriterTest, SimpleWrite) {
  auto chunk = Chunk::FromString("test");
  auto result = testee_->Write(chunk.get());
  EXPECT_EQ(4, result.n());
  result = testee_->Write(chunk.get());
  EXPECT_EQ(4, result.n());
  EXPECT_EQ(8, testee_->total_size());

  auto written = testee_->ReleaseChunks();
  EXPECT_EQ(0, testee_->total_size());
  std::string written_data;
  for (const auto& chunk : written) {
    written_data += chunk->ToString().as_string();
  }
  EXPECT_EQ("testtest", written_data);

  result = testee_->Write(chunk.get());
  EXPECT_EQ(4, result.n());
  written = testee_->ReleaseChunks();
  written_data.clear();
  for (const auto& chunk : written) {
    written_data += chunk->ToString().as_string();
  }
  EXPECT_EQ("test", written_data);
}

TEST_F(BufferWriterTest, Unwriting) {
  auto chunk1 = Chunk::FromString("test");
  auto chunk2 = Chunk::FromString("meagain");
  auto chunk3 = Chunk::FromString("andagain");
  auto result = testee_->Write(chunk1.get());
  result = testee_->Write(chunk2.get());
  auto unwritten = testee_->UnwriteN(3);
  EXPECT_EQ(3, unwritten);
  result = testee_->Write(chunk3.get());

  auto written = testee_->ReleaseChunks();
  std::string written_data;
  for (const auto& chunk : written) {
    written_data += chunk->ToString().as_string();
  }
  EXPECT_EQ("testmeagandagain", written_data);
}

}  // namespace io
