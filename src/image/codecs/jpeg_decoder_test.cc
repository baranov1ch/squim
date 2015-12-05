#include "image/codecs/jpeg_decoder.h"

#include <memory>

#include "gtest/gtest.h"

namespace image {

namespace {

JpegFileReader{};

}  // namespace

class JpegDecoderTest : public testing::Test {
 protected:
  std::unique_ptr<JpegDecoder> testee_;
};

TEST_F(JpegDecoderTest, ReadSuccess) {}

}  // namespace image
