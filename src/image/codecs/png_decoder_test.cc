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

#include "image/codecs/png_decoder.h"

#include <memory>
#include <vector>

#include "base/memory/make_unique.h"
#include "image/image_test_util.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char* kValidPngSuiteFiles[]{
    "basi0g01",   "basi0g02",    "basi0g04",   "basi0g08",  "basi0g16",
    "basi2c08",   "basi2c16",    "basi3p01",   "basi3p02",  "basi3p04",
    "basi3p08",   "basi4a08",    "basi4a16",   "basi6a08",  "basi6a16",
    "basn0g01",   "basn0g02",    "basn0g04",   "basn0g08",  "basn0g16",
    "basn2c08",   "basn2c16",    "basn3p01",   "basn3p02",  "basn3p04",
    "basn3p08",   "basn4a08",    "basn4a16",   "basn6a08",  "basn6a16",
    "bgai4a08",   "bgai4a16",    "bgan6a08",   "bgan6a16",  "bgbn4a08",
    "bggn4a16",   "bgwn6a08",    "bgyn6a16",   "ccwn2c08",  "ccwn3p08",
    "cdfn2c08",   "cdhn2c08",    "cdsn2c08",   "cdun2c08",  "ch1n3p04",
    "ch2n3p08",   "cm0n0g04",    "cm7n0g04",   "cm9n0g04",  "cs3n2c16",
    "cs3n3p08",   "cs5n2c08",    "cs5n3p08",   "cs8n2c08",  "cs8n3p08",
    "ct0n0g04",   "ct1n0g04",    "ctzn0g04",   "f00n0g08",  "f00n2c08",
    "f01n0g08",   "f01n2c08",    "f02n0g08",   "f02n2c08",  "f03n0g08",
    "f03n2c08",   "f04n0g08",    "f04n2c08",   "g03n0g16",  "g03n2c08",
    "g03n3p04",   "g04n0g16",    "g04n2c08",   "g04n3p04",  "g05n0g16",
    "g05n2c08",   "g05n3p04",    "g07n0g16",   "g07n2c08",  "g07n3p04",
    "g10n0g16",   "g10n2c08",    "g10n3p04",   "g25n0g16",  "g25n2c08",
    "g25n3p04",   "oi1n0g16",    "oi1n2c16",   "oi2n0g16",  "oi2n2c16",
    "oi4n0g16",   "oi4n2c16",    "oi9n0g16",   "oi9n2c16",  "pp0n2c16",
    "pp0n6a08",   "ps1n0g08",    "ps1n2c16",   "ps2n0g08",  "ps2n2c16",
    "s01i3p01",   "s01n3p01",    "s02i3p01",   "s02n3p01",  "s03i3p01",
    "s03n3p01",   "s04i3p01",    "s04n3p01",   "s05i3p02",  "s05n3p02",
    "s06i3p02",   "s06n3p02",    "s07i3p02",   "s07n3p02",  "s08i3p02",
    "s08n3p02",   "s09i3p02",    "s09n3p02",   "s32i3p04",  "s32n3p04",
    "s33i3p04",   "s33n3p04",    "s34i3p04",   "s34n3p04",  "s35i3p04",
    "s35n3p04",   "s36i3p04",    "s36n3p04",   "s37i3p04",  "s37n3p04",
    "s38i3p04",   "s38n3p04",    "s39i3p04",   "s39n3p04",  "s40i3p04",
    "s40n3p04",   "tbbn1g04",    "tbbn2c16",   "tbbn3p08",  "tbgn2c16",
    "tbgn3p08",   "tbrn2c08",    "tbwn1g16",   "tbwn3p08",  "tbyn3p08",
    "tp0n1g08",   "tp0n2c08",    "tp0n3p08",   "tp1n3p08",  "tr-basi4a08",
    "tm3n3p02",   "tr-basn4a08", "tr-t1-8pB",  "tr-t2-8pb", "tr-t3-32pB",
    "tr-t4-32pb", "tr-t5-64pB",  "tr-t6-64pb", "z00n2c08",  "z03n2c08",
    "z06n2c08",   "z09n2c08",
};

const char* kInvalidPngFiles[] = {
    "emptyfile", "x00n0g01", "xcrn0g04", "xlfn0g04",
};

const char kPngSuiteDir[] = "pngsuite";

std::unique_ptr<ImageDecoder> CreateDecoder(
    std::unique_ptr<io::BufReader> source) {
  auto decoder = base::make_unique<PngDecoder>(PngDecoder::Params::Default(),
                                               std::move(source));
  EXPECT_EQ(ImageType::kPng, decoder->GetImageType());
  return std::move(decoder);
}

}  // namespace

class PngDecoderTest : public testing::Test {
 protected:
  void ValidatePngRandomReads(const std::string& filename,
                              size_t max_chunk_size,
                              ReadType read_type) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFile(kPngSuiteDir, filename, "png", &data));
    auto read_spec = GenerateFuzzyReads(data.size(), max_chunk_size);
    auto ref_reader = [&data, filename](ImageInfo* info,
                                        ImageFrame* frame) -> bool {
      return LoadReferencePng(filename, data, info, frame);
    };
    ValidateDecodeWithReadSpec(filename, data, CreateDecoder, ref_reader,
                               read_spec, read_type);
  }

  void CheckInvalidRead(const std::string& filename) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFile(kPngSuiteDir, filename, "png", &data));
    auto source = base::make_unique<io::BufReader>(
        base::make_unique<io::BufferedSource>());
    source->source()->AddChunk(
        base::make_unique<io::Chunk>(&data[0], data.size()));
    source->source()->SendEof();
    auto testee = base::make_unique<PngDecoder>(PngDecoder::Params::Default(),
                                                std::move(source));
    auto result = testee->Decode();
    if (filename == "emptyfile") {
      EXPECT_EQ(Result::Code::kUnexpectedEof, result.code()) << filename;
    } else {
      EXPECT_EQ(Result::Code::kDecodeError, result.code()) << filename;
    }
  }
};

TEST_F(PngDecoderTest, DecoderError) {
  for (auto pic : kInvalidPngFiles)
    CheckInvalidRead(pic);
}

TEST_F(PngDecoderTest, ReadHeaderOnly) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 0, ReadType::kReadHeaderOnly);
}

TEST_F(PngDecoderTest, ReadHeaderOnlySmall) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 5, ReadType::kReadHeaderOnly);
}

TEST_F(PngDecoderTest, ReadHeaderThenRest) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 0, ReadType::kReadHeaderThenBody);
}

TEST_F(PngDecoderTest, ReadAll) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 0, ReadType::kReadAll);
}

TEST_F(PngDecoderTest, ReadAllSmall) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 10, ReadType::kReadAll);
}

TEST_F(PngDecoderTest, ReadAllMedium) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 100, ReadType::kReadAll);
}

TEST_F(PngDecoderTest, ReadHeaderThenRestSmall) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 10, ReadType::kReadHeaderThenBody);
}

TEST_F(PngDecoderTest, ReadHeaderThenRestMedium) {
  for (auto pic : kValidPngSuiteFiles)
    ValidatePngRandomReads(pic, 100, ReadType::kReadHeaderThenBody);
}

TEST_F(PngDecoderTest, ReadExpandGray) {
  auto decoder_builder = [](
      std::unique_ptr<io::BufReader> source) -> std::unique_ptr<ImageDecoder> {
    PngDecoder::Params params;
    params.allowed_color_schemes.insert(ColorScheme::kRGB);
    params.allowed_color_schemes.insert(ColorScheme::kRGBA);
    auto decoder = base::make_unique<PngDecoder>(params, std::move(source));
    EXPECT_EQ(ImageType::kPng, decoder->GetImageType());
    return std::move(decoder);
  };
  const char* kSomeGrayImages[] = {
      "basi0g01", "basi0g02", "basi0g04", "basi0g08", "basi0g16",
  };
  for (const auto& filename : kSomeGrayImages) {
    std::vector<uint8_t> data;
    ASSERT_TRUE(ReadTestFile(kPngSuiteDir, filename, "png", &data));
    auto read_spec = GenerateFuzzyReads(data.size(), 1000);
    auto ref_reader = [&data, filename](ImageInfo* info,
                                        ImageFrame* frame) -> bool {
      return LoadReferencePngExpandGray(filename, data, true, info, frame);
    };
    ValidateDecodeWithReadSpec(filename, data, decoder_builder, ref_reader,
                               read_spec, ReadType::kReadAll);
  }
}

}  // namespace image
