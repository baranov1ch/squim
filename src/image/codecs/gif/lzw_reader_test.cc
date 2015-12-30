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

#include "image/codecs/gif/lzw_reader.h"
#include "image/codecs/gif/lzw_writer.h"

#include <vector>

#include "gtest/gtest.h"

namespace image {

TEST(LZWReaderTest, Success) {
  std::vector<uint8_t> input;
  input.reserve(2000);
  for (size_t i = 0; i < 2000; ++i) {
    input.push_back(static_cast<uint8_t>(i % 32));
  }

  ASSERT_EQ(2000, input.size());

  std::vector<uint8_t> encoded;
  LZWWriter writer;
  ASSERT_TRUE(writer.Init(5, 128, [&encoded](uint8_t* data, size_t len) -> bool {
    encoded.insert(encoded.end(), data, data + len);
    return true;
  }));
  auto result = writer.Write(&input[0], input.size());
  EXPECT_TRUE(result.ok());
  EXPECT_EQ(2000, result.n());
  EXPECT_LT(encoded.size(), input.size());
}

}  // namespace image
