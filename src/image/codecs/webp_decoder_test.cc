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

#include "image/codecs/webp_decoder.h"

#include <memory>
#include <vector>

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/test/image_test_util.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char kWebPTestDir[] = "webp";

std::unique_ptr<ImageDecoder> CreateDecoder(
    std::unique_ptr<io::BufReader> source) {
  auto decoder = base::make_unique<WebPDecoder>(WebPDecoder::Params::Default(),
                                                std::move(source));
  EXPECT_EQ(ImageType::kWebP, decoder->GetImageType());
  return std::move(decoder);
}

}  // namespace

class WebPDecoderTest : public testing::Test {
 protected:
  void ValidateWebPRandomReads(const std::string& filename,
                               size_t max_chunk_size,
                               ReadType read_type) {
    std::vector<uint8_t> webp_data;
    std::vector<uint8_t> png_data;
    ASSERT_TRUE(ReadTestFile(kWebPTestDir, filename, "webp", &webp_data));
    ASSERT_TRUE(ReadTestFile(kWebPTestDir, filename, "png", &png_data));
    auto read_spec = GenerateFuzzyReads(webp_data.size(), max_chunk_size);
    auto ref_reader = [&png_data, filename](ImageInfo* info,
                                            ImageFrame* frame) -> bool {
      return LoadReferencePng(filename, png_data, info, frame);
    };
    ValidateDecodeWithReadSpec(filename, webp_data, CreateDecoder, ref_reader,
                               read_spec, read_type);
  }

  void CheckInvalidRead(const std::string& filename) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFileWithExt(kWebPTestDir, filename, &data));
    auto source = base::make_unique<io::BufReader>(
        base::make_unique<io::BufferedSource>());
    source->source()->AddChunk(
        base::make_unique<io::Chunk>(&data[0], data.size()));
    source->source()->SendEof();
    auto testee = base::make_unique<WebPDecoder>(WebPDecoder::Params::Default(),
                                                 std::move(source));
    auto result = testee->Decode();
    EXPECT_EQ(Result::Code::kDecodeError, result.code()) << filename;
  }
};

TEST_F(WebPDecoderTest, ReadSuccessAll) {}

}  // namespace image
