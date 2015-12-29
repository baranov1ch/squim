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

#include "io/buf_reader.h"

#include <memory>

#include "base/memory/make_unique.h"
#include "io/buffered_source.h"
#include "io/chunk.h"

#include "gtest/gtest.h"

namespace io {

namespace {
std::string StringFromBytes(uint8_t* bytes, uint64_t len) {
  auto* chars = reinterpret_cast<const char*>(bytes);
  return std::string(chars, len);
}
}

class BufReaderTest : public ::testing::Test {
 public:
  void SetUp() override { testee_ = BufReader::CreateEmpty(); }

 protected:
  std::unique_ptr<BufReader> testee_;
};

TEST_F(BufReaderTest, ReadPending) {
  uint8_t* out;
  EXPECT_TRUE(testee_->ReadSome(&out).pending());
  EXPECT_TRUE(testee_->ReadAtMostN(&out, 100).pending());
  EXPECT_TRUE(testee_->ReadN(&out, 100).pending());

  std::unique_ptr<uint8_t[]> buf(new uint8_t[20]);
  EXPECT_TRUE(testee_->ReadNInto(buf.get(), 20).pending());
  EXPECT_TRUE(testee_->PeekNInto(buf.get(), 20).pending());
}

TEST_F(BufReaderTest, ReadEOF) {
  testee_->source()->SendEof();
  uint8_t* out;
  EXPECT_TRUE(testee_->ReadSome(&out).eof());
  EXPECT_TRUE(testee_->ReadAtMostN(&out, 100).eof());
  EXPECT_TRUE(testee_->ReadN(&out, 100).eof());

  std::unique_ptr<uint8_t[]> buf(new uint8_t[20]);
  EXPECT_TRUE(testee_->ReadNInto(buf.get(), 20).eof());
  EXPECT_TRUE(testee_->PeekNInto(buf.get(), 20).eof());
}

TEST_F(BufReaderTest, Reading) {
  testee_->source()->AddChunk(base::make_unique<StringChunk>("test1"));
  uint8_t* out;
  EXPECT_EQ(5, testee_->ReadSome(&out).n());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
  EXPECT_EQ(5, testee_->UnreadN(5));

  EXPECT_EQ(5, testee_->ReadAtMostN(&out, 10).n());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
  EXPECT_EQ(5, testee_->UnreadN(5));

  EXPECT_EQ(5, testee_->ReadN(&out, 5).n());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
  EXPECT_EQ(5, testee_->UnreadN(5));

  std::unique_ptr<uint8_t[]> buf(new uint8_t[10]);
  EXPECT_EQ(5, testee_->PeekNInto(out, 5).n());
  EXPECT_EQ("test1", StringFromBytes(out, 5));

  EXPECT_EQ(5, testee_->ReadNInto(out, 5).n());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
}

}  // namespace io
