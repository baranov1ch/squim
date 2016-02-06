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

#include "squim/image/codecs/gif/lzw_reader.h"

#include <vector>

#include "squim/image/codecs/gif/lzw_writer.h"

#include "gtest/gtest.h"

namespace image {

class LZWReaderTest : public testing::Test {
 protected:
  void CheckEncodeDecodeCycle(const std::vector<uint8_t>& input,
                              size_t data_size,
                              size_t chunk_size) {
    std::vector<uint8_t> encoded;
    LZWWriter writer;
    ASSERT_TRUE(writer.Init(data_size, chunk_size,
                            [&encoded](uint8_t* data, size_t len) -> bool {
                              encoded.insert(encoded.end(), data, data + len);
                              return true;
                            }));
    auto result = writer.Write(&input[0], input.size());
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(input.size(), result.n());
    EXPECT_LT(encoded.size(), input.size());
    ASSERT_TRUE(writer.Finish().ok());

    std::vector<uint8_t> decoded;
    LZWReader reader;
    ASSERT_TRUE(reader.Init(data_size, chunk_size,
                            [&decoded](uint8_t* data, size_t len) -> bool {
                              decoded.insert(decoded.end(), data, data + len);
                              return true;
                            }));
    result = reader.Decode(&encoded[0], encoded.size());
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(encoded.size(), result.n());
    EXPECT_GT(decoded.size(), encoded.size());

    EXPECT_EQ(input.size(), decoded.size());
    EXPECT_EQ(input, decoded);
  }
};

TEST_F(LZWReaderTest, SuccessReference) {
  // Image from
  // http://www.matthewflickinger.com/lab/whatsinagif/lzw_image_data.asp.
  std::vector<uint8_t> input{{
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      1, 1, 1, 0, 0, 0, 0, 2, 2, 2,
      1, 1, 1, 0, 0, 0, 0, 2, 2, 2,
      1, 1, 1, 0, 0, 0, 0, 2, 2, 2,
      1, 1, 1, 0, 0, 0, 0, 2, 2, 2,
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
  }};
  CheckEncodeDecodeCycle(input, 2, 32);
}

TEST_F(LZWReaderTest, SuccessLarge) {
  std::vector<uint8_t> input;
  size_t kTestSize = 100000;
  input.reserve(kTestSize);
  for (size_t i = 0; i < kTestSize; ++i)
    input.push_back(i % 256);

  ASSERT_EQ(kTestSize, input.size());

  CheckEncodeDecodeCycle(input, 8, 1024);
}

}  // namespace image
