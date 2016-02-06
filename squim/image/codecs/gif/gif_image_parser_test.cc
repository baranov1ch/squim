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

#include "squim/image/codecs/gif/gif_image_parser.h"

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/test/image_test_util.h"
#include "squim/io/buf_reader.h"
#include "squim/io/chunk.h"

#include "gtest/gtest.h"

namespace image {

namespace {
const char kPngSuiteGifDir[] = "pngsuite/gif";
}

class GifImageParserTest : public ::testing::Test {
 public:
  GifImageParserTest() {
    reader_ = io::BufReader::CreateEmpty();
    parser_ = base::make_unique<GifImage::Parser>(reader_.get(), &image_);
  }

 protected:
  GifImage image_;
  std::unique_ptr<io::BufReader> reader_;
  std::unique_ptr<GifImage::Parser> parser_;
};

TEST_F(GifImageParserTest, ParseOneImage) {
  std::vector<uint8_t> data;
  ASSERT_TRUE(ReadTestFile(kPngSuiteGifDir, "basi0g01", "gif", &data));
  reader_->source()->AddChunk(io::Chunk::View(&data[0], data.size()));
  auto result = parser_->Parse();
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(87, image_.version());
  EXPECT_EQ(32, image_.screen_width());
  EXPECT_EQ(32, image_.screen_height());
  const auto* color_table = image_.global_color_table();
  ASSERT_TRUE(color_table);
  EXPECT_EQ(2, color_table->expected_size());
  auto pixel1 = color_table->GetColor(0);
  auto pixel2 = color_table->GetColor(1);
  EXPECT_EQ(0, pixel1.r());
  EXPECT_EQ(0, pixel1.g());
  EXPECT_EQ(0, pixel1.b());
  EXPECT_EQ(255, pixel2.r());
  EXPECT_EQ(255, pixel2.g());
  EXPECT_EQ(255, pixel2.b());

  const auto& frames = image_.frames();
  EXPECT_EQ(1, frames.size());
  const auto& frame = frames[0];
  EXPECT_EQ(32, frame->width());
  EXPECT_EQ(32, frame->height());
}

}  // namespace image
