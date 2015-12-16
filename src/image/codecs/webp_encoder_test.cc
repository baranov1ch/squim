#include "image/codecs/webp_encoder.h"

#include <memory>
#include <vector>

#include "base/memory/make_unique.h"
#include "image/image_frame.h"
#include "image/image_info.h"
#include "image/image_test_util.h"
#include "io/writer.h"
#include "glog/logging.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char kWebPTestDir[] = "webp";

class TestWriter : public io::VectorWriter {
 public:
  io::IoResult WriteV(io::ChunkList chunks) override {
    for (const auto& chunk : chunks)
      data_.insert(data_.end(), chunk->data(), chunk->data() + chunk->size());

    return io::IoResult::Read(5);
  }

  std::vector<uint8_t>& data() { return data_; }

 private:
  std::vector<uint8_t> data_;
};
}

class WebPEncoderTest : public testing::Test {};

TEST_F(WebPEncoderTest, Success) {
  auto writer = base::make_unique<TestWriter>();
  auto* writer_raw = writer.get();
  std::vector<uint8_t> png_data;
  ImageInfo info;
  ImageFrame frame;
  ASSERT_TRUE(ReadTestFile(kWebPTestDir, "alpha_32x32", "png", &png_data));
  ASSERT_TRUE(LoadReferencePng("alpha_32x32", png_data, &info, &frame));
  WebPEncoder encoder(WebPEncoder::Params(), std::move(writer));
  auto result = encoder.EncodeFrame(&frame, true);
  EXPECT_EQ(Result::Code::kOk, result.code());
  LOG(INFO) << writer_raw->data().size();
  LOG(INFO) << png_data.size();
}

}  // namespace image
