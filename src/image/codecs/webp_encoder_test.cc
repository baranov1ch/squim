#include "image/codecs/webp_encoder.h"

#include <memory>
#include <vector>

#include "base/memory/make_unique.h"
#include "image/image_frame.h"
#include "image/image_info.h"
#include "image/image_test_util.h"
#include "io/writer.h"
#include "glog/logging.h"
#include "google/libwebp/upstream/src/webp/decode.h"

#include "gtest/gtest.h"

namespace image {

namespace {

const char kWebPTestDir[] = "webp";

const char* kValidImages[] = {
    "alpha_32x32", "opaque_32x20",
};

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

bool ReadWebP(const std::vector<uint8_t>& data,
              uint32_t width,
              uint32_t height,
              ColorScheme color_scheme,
              ImageFrame* frame) {
  WebPDecoderConfig config;
  if (!WebPInitDecoderConfig(&config))
    return false;

  frame->Init(width, height, color_scheme);

  if (color_scheme == ColorScheme::kRGB) {
    config.output.colorspace = MODE_RGB;
  } else {
    config.output.colorspace = MODE_RGBA;
  }

  config.output.u.RGBA.rgba = frame->GetData(0);
  config.output.u.RGBA.stride = frame->stride();
  config.output.u.RGBA.size = frame->stride() * frame->height();
  config.output.is_external_memory = true;

  auto result = WebPDecode(&data[0], data.size(), &config);
  WebPFreeDecBuffer(&config.output);
  return result == VP8_STATUS_OK;
}

}  // namespace

class WebPEncoderTest : public testing::Test {
 protected:
  void ValidateEncoding(const std::string filename) {
    auto writer = base::make_unique<TestWriter>();
    auto* writer_raw = writer.get();
    std::vector<uint8_t> png_data;
    ImageInfo info;
    ImageFrame ref_frame;
    ASSERT_TRUE(ReadTestFile(kWebPTestDir, filename, "png", &png_data))
        << filename;
    ASSERT_TRUE(LoadReferencePng(filename, png_data, &info, &ref_frame))
        << filename;
    WebPEncoder::Params params;
    params.quality = 90;
    auto testee = base::make_unique<WebPEncoder>(params, std::move(writer));
    auto result = testee->EncodeFrame(&ref_frame, true);
    EXPECT_EQ(Result::Code::kOk, result.code());
    ImageFrame webp_frame;
    ASSERT_TRUE(ReadWebP(writer_raw->data(), info.width, info.height,
                         info.color_scheme, &webp_frame))
        << filename;
    CheckImageFrameByPSNR(filename, &ref_frame, &webp_frame, 33);
  }
};

TEST_F(WebPEncoderTest, Success) {
  for (auto pic : kValidImages)
    ValidateEncoding(pic);
}

}  // namespace image
