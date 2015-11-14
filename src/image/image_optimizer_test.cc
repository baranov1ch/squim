#include "image/image_optimizer.h"

#include "gtest/gtest.h"

namespace image {

TEST(ImageOptimizerTest, ImageTypeChoice) {
  struct {
    const char data[15];
    ImageType expected_type;
  } kCases[] = {
    { "\xFF\xD8\xFF""sflj123jioj", ImageType::kJpeg },
    { "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A""23jioj", ImageType::kPng },
    { "GIF87alj123jio", ImageType::kGif },
    { "GIF89alj123jio", ImageType::kGif },
    { "RIFF?!@?WEBPVP", ImageType::kWebP },
    { "lkjsnw1284jsoi", ImageType::kUnknown },
  };

  for (const auto c : kCases) {
    auto sig = reinterpret_cast<const uint8_t*>(c.data);
    EXPECT_EQ(c.expected_type, ImageOptimizer::ChooseImageType(sig));
  }
}

}  // namespace image
