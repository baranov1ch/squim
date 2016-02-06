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

#include "squim/image/codecs/jpeg_decoder.h"

#include <memory>
#include <vector>

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/image/image_info.h"
#include "squim/image/test/image_test_util.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char kJpegTestDir[] = "jpeg";

const char* kValidJpegImages[] = {
    "test411",   // RGB color space with 4:1:1 chroma sub-sampling.
    "test420",   // RGB color space with 4:2:0 chroma sub-sampling.
    "test422",   // RGB color space with 4:2:2 chroma sub-sampling.
    "test444",   // RGB color space with full chroma information.
    "testgray",  // Grayscale color space.
};

const char* kInvalidFiles[] = {
    "notajpeg.png",   // A png.
    "notajpeg.gif",   // A gif.
    "emptyfile.jpg",  // A zero-byte file.
    "corrupt.jpg",    // Invalid huffman code in the image data section.
};

std::unique_ptr<ImageDecoder> CreateDecoder(
    std::unique_ptr<io::BufReader> source) {
  auto decoder = base::make_unique<JpegDecoder>(JpegDecoder::Params::Default(),
                                                std::move(source));
  return std::move(decoder);
}

}  // namespace

class JpegDecoderTest : public testing::Test {
 protected:
  void ValidateJpegRandomReads(const std::string& filename,
                               size_t max_chunk_size,
                               ReadType read_type) {
    std::vector<uint8_t> jpeg_data;
    std::vector<uint8_t> png_data;
    ASSERT_TRUE(ReadTestFile(kJpegTestDir, filename, "jpg", &jpeg_data));
    ASSERT_TRUE(ReadTestFile(kJpegTestDir, filename, "png", &png_data));
    auto read_spec = GenerateFuzzyReads(jpeg_data.size(), max_chunk_size);
    auto ref_reader = [&png_data, filename](ImageInfo* info,
                                            ImageFrame* frame) -> bool {
      return LoadReferencePng(filename, png_data, info, frame);
    };
    ValidateDecodeWithReadSpec(filename, jpeg_data, CreateDecoder, ref_reader,
                               read_spec, read_type);
  }

  void CheckInvalidRead(const std::string& filename) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFileWithExt(kJpegTestDir, filename, &data));
    auto source = base::make_unique<io::BufReader>(
        base::make_unique<io::BufferedSource>());
    source->source()->AddChunk(
        base::make_unique<io::Chunk>(&data[0], data.size()));
    source->source()->SendEof();
    auto testee = base::make_unique<JpegDecoder>(JpegDecoder::Params::Default(),
                                                 std::move(source));
    auto result = testee->Decode();
    if (filename == "emptyfile.jpg") {
      EXPECT_EQ(Result::Code::kUnexpectedEof, result.code()) << filename;
    } else {
      EXPECT_EQ(Result::Code::kDecodeError, result.code()) << filename;
    }
  }
};

TEST_F(JpegDecoderTest, ReadSuccessAll) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 0, ReadType::kReadAll);
}

TEST_F(JpegDecoderTest, ReadInvalidJpeg) {
  for (auto pic : kInvalidFiles)
    CheckInvalidRead(pic);
}

TEST_F(JpegDecoderTest, ReadHeaderOnly) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 0, ReadType::kReadHeaderOnly);
}

TEST_F(JpegDecoderTest, ReadHeaderThenRest) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 0, ReadType::kReadHeaderThenBody);
}

TEST_F(JpegDecoderTest, ReadAll) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 0, ReadType::kReadAll);
}

TEST_F(JpegDecoderTest, ReadHeaderOnlySmall) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 10, ReadType::kReadHeaderOnly);
}

TEST_F(JpegDecoderTest, ReadHeaderOnlyMedium) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 100, ReadType::kReadHeaderOnly);
}

TEST_F(JpegDecoderTest, ReadAllSmall) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 10, ReadType::kReadAll);
}

TEST_F(JpegDecoderTest, ReadAllMedium) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 100, ReadType::kReadAll);
}

TEST_F(JpegDecoderTest, ReadHeaderThenRestSmall) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 10, ReadType::kReadHeaderThenBody);
}

TEST_F(JpegDecoderTest, ReadHeaderThenRestMedium) {
  for (auto pic : kValidJpegImages)
    ValidateJpegRandomReads(pic, 100, ReadType::kReadHeaderThenBody);
}

TEST_F(JpegDecoderTest, ReadExpandGrayScale) {
  auto decoder_builder = [](
      std::unique_ptr<io::BufReader> source) -> std::unique_ptr<ImageDecoder> {
    JpegDecoder::Params params;
    params.allowed_color_schemes.insert(ColorScheme::kRGB);
    params.allowed_color_schemes.insert(ColorScheme::kRGBA);
    auto decoder = base::make_unique<JpegDecoder>(params, std::move(source));
    return std::move(decoder);
  };
  std::string filename("testgray");
  std::vector<uint8_t> jpeg_data;
  std::vector<uint8_t> png_data;
  ASSERT_TRUE(ReadTestFile(kJpegTestDir, filename, "jpg", &jpeg_data));
  ASSERT_TRUE(ReadTestFile(kJpegTestDir, filename, "png", &png_data));
  auto read_spec = GenerateFuzzyReads(jpeg_data.size(), 1000);
  auto ref_reader = [&png_data, filename](ImageInfo* info,
                                          ImageFrame* frame) -> bool {
    return LoadReferencePngExpandGray(filename, png_data, true, info, frame);
  };
  ValidateDecodeWithReadSpec(filename, jpeg_data, decoder_builder, ref_reader,
                             read_spec, ReadType::kReadHeaderOnly);
}

}  // namespace image
