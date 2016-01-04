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

#include "image/codecs/gif_decoder.h"

#include <memory>
#include <vector>

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/test/image_test_util.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char kGifTestDir[] = "gif";
const char kPngSuiteGifDir[] = "pngsuite/gif";
const char kPngSuiteDir[] = "pngsuite";

const char* kValidPngSuiteFiles[]{
    "basi0g01", "basi0g02", "basi0g04", "basi0g08", "basi3p01", "basi3p02",
    "basi3p04", "basi3p08", "basn0g01", "basn0g02", "basn0g04", "basn0g08",
    "basn3p01", "basn3p02", "basn3p04", "basn3p08",
};

std::unique_ptr<ImageDecoder> CreateDecoder(
    std::unique_ptr<io::BufReader> source) {
  auto decoder = base::make_unique<GifDecoder>(GifDecoder::Params::Default(),
                                               std::move(source));
  EXPECT_EQ(ImageType::kGif, decoder->GetImageType());
  return std::move(decoder);
}

}  // namespace

class GifDecoderTest : public testing::Test {
 protected:
  void ValidateGifRandomReads(const std::string& filename,
                              size_t max_chunk_size,
                              ReadType read_type) {
    ValidateGifRandomReadsFromDir(filename, kPngSuiteGifDir, kPngSuiteDir,
                                  max_chunk_size, read_type);
  }

  void ValidateGifRandomReadsFromDir(const std::string& filename,
                                     const std::string& gif_dir,
                                     const std::string& png_dir,
                                     size_t max_chunk_size,
                                     ReadType read_type) {
    std::vector<uint8_t> gif_data;
    std::vector<uint8_t> png_data;
    ASSERT_TRUE(ReadTestFile(gif_dir, filename, "gif", &gif_data));
    ASSERT_TRUE(ReadTestFile(png_dir, filename, "png", &png_data));
    auto read_spec = GenerateFuzzyReads(gif_data.size(), max_chunk_size);
    auto ref_reader = [&png_data, filename](ImageInfo* info,
                                            ImageFrame* frame) -> bool {
      return LoadReferencePngExpandGray(filename, png_data, true, info, frame);
    };
    ValidateDecodeWithReadSpec(filename, gif_data, CreateDecoder, ref_reader,
                               read_spec, read_type);
  }

  void CheckInvalidRead(const std::string& filename) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFileWithExt(kGifTestDir, filename, &data));
    auto source = base::make_unique<io::BufReader>(
        base::make_unique<io::BufferedSource>());
    source->source()->AddChunk(
        base::make_unique<io::Chunk>(&data[0], data.size()));
    source->source()->SendEof();
    auto testee = base::make_unique<GifDecoder>(GifDecoder::Params::Default(),
                                                std::move(source));
    auto result = testee->Decode();
    EXPECT_EQ(Result::Code::kDecodeError, result.code()) << filename;
  }
};

TEST_F(GifDecoderTest, ReadAll) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 0, ReadType::kReadAll);
}

TEST_F(GifDecoderTest, ReadHeaderOnly) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 0, ReadType::kReadHeaderOnly);
}

TEST_F(GifDecoderTest, ReadHeaderThenRest) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 0, ReadType::kReadHeaderThenBody);
}

TEST_F(GifDecoderTest, ReadAllSmall) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 10, ReadType::kReadAll);
}

TEST_F(GifDecoderTest, ReadHeaderOnlySmall) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 5, ReadType::kReadHeaderOnly);
}

TEST_F(GifDecoderTest, ReadHeaderThenRestSmall) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 10, ReadType::kReadHeaderThenBody);
}

TEST_F(GifDecoderTest, ReadAllMedium) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 100, ReadType::kReadAll);
}

TEST_F(GifDecoderTest, ReadHeaderOnlyMedium) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 50, ReadType::kReadHeaderOnly);
}

TEST_F(GifDecoderTest, ReadHeaderThenRestMedium) {
  for (auto pic : kValidPngSuiteFiles)
    ValidateGifRandomReads(pic, 100, ReadType::kReadHeaderThenBody);
}

TEST_F(GifDecoderTest, ReadInterlaced) {
  ValidateGifRandomReadsFromDir("interlaced", kGifTestDir, kGifTestDir, 50,
                                ReadType::kReadAll);
}

TEST_F(GifDecoderTest, ReadAnimated) {
  const char* kAnimated[] = {
      "animated", "animated_interlaced",
  };
  for (auto pic : kAnimated) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFile(kGifTestDir, pic, "gif", &data));
    auto source = base::make_unique<io::BufReader>(
        base::make_unique<io::BufferedSource>());
    source->source()->AddChunk(
        base::make_unique<io::Chunk>(&data[0], data.size()));
    source->source()->SendEof();
    auto testee = base::make_unique<GifDecoder>(GifDecoder::Params::Default(),
                                                std::move(source));
    auto result = testee->Decode();
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(8, testee->GetFrameCount());
  }
}

}  // namespace image
