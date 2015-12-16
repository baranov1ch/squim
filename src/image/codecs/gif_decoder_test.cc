#include "image/codecs/gif_decoder.h"

#include <memory>
#include <vector>

#include "base/memory/make_unique.h"
#include "image/image_test_util.h"
#include "glog/logging.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char kGifTestDir[] = "gif";

std::unique_ptr<ImageDecoder> CreateDecoder(
    std::unique_ptr<io::BufReader> source) {
  auto decoder = base::make_unique<GifDecoder>(std::move(source));
  EXPECT_EQ(ImageType::kGif, decoder->GetImageType());
  return std::move(decoder);
}

}  // namespace

class GifDecoderTest : public testing::Test {
 protected:
  void ValidateGifRandomReads(const std::string& filename,
                              size_t max_chunk_size,
                              ReadType read_type) {
    std::vector<uint8_t> gif_data;
    std::vector<uint8_t> png_data;
    ASSERT_TRUE(ReadTestFile(kGifTestDir, filename, "gif", &gif_data));
    ASSERT_TRUE(ReadTestFile(kGifTestDir, filename, "png", &png_data));
    auto read_spec = GenerateFuzzyReads(gif_data.size(), max_chunk_size);
    auto ref_reader = [&png_data, filename](ImageInfo* info,
                                            ImageFrame* frame) -> bool {
      return LoadReferencePng(filename, png_data, info, frame);
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
    auto testee = base::make_unique<GifDecoder>(std::move(source));
    auto result = testee->Decode();
    EXPECT_EQ(Result::Code::kDecodeError, result.code()) << filename;
  }
};

TEST_F(GifDecoderTest, ReadSuccessAll) {}

}  // namespace image
