#ifndef IMAGE_CODECS_WEBP_ENCODER_H_
#define IMAGE_CODECS_WEBP_ENCODER_H_

#include <memory>

#include "base/make_noncopyable.h"
#include "image/image_encoder.h"
#include "io/chunk.h"

namespace io {
class VectorWriter;
}

namespace image {

class WebPEncoder : public ImageEncoder {
  MAKE_NONCOPYABLE(WebPEncoder);

 public:
  enum class Preset {
    kDefault,
    kPhoto,
    kPicture,
    kDrawing,
    kIcon,
    kText,
  };

  enum class Hint {
    kDefault,
    kPicture,
    kPhoto,
    kGraph,
  };

  enum class Compression {
    kLossy,
    kLossless,
    kMixed,
  };

  struct Params {
    float quality = 50.0;
    // Set WebP compression method to 3 (4 is the default). From
    // third_party/libwebp/v0_2/src/webp/encode.h, the method determines the
    // 'quality/speed trade-off (0=fast, 6=slower-better). On a representative
    // set of images, we see a 26% improvement in the 75th percentile
    // compression time, even greater improvements further along the tail, and
    // no increase in file size. Method 2 incurs a prohibitive 10% increase in
    // file size, which is not worth the compression time savings.
    int method = 3;
    Hint hint = Hint::kDefault;
    Preset preset = Preset::kDefault;
    Compression compression = Compression::kLossy;
  };

  WebPEncoder(Params params, std::unique_ptr<io::VectorWriter> dst);
  ~WebPEncoder() override;

  // ImageEncoder implementation:
  Result EncodeFrame(ImageFrame* frame, bool last_frame) override;
  void SetMetadata(const ImageMetadata* metadata) override;
  Result FinishWrite() override;

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
  Params params_;
  std::unique_ptr<io::VectorWriter> dst_;
  const ImageMetadata* metadata_;
  io::ChunkList output_;
};

}  // namespace image

#endif  // IMAGE_CODECS_WEBP_ENCODER_H_
