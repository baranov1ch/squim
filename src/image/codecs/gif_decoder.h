#ifndef IMAGE_CODECS_GIF_DECODER_H_
#define IMAGE_CODECS_GIF_DECODER_H_

#include <memory>
#include <vector>

#include "base/make_noncopyable.h"
#include "image/codecs/decode_params.h"
#include "image/image_decoder.h"
#include "image/image_frame.h"
#include "image/image_metadata.h"

namespace io {
class BufReader;
}

namespace image {

class GifDecoder : public ImageDecoder {
  MAKE_NONCOPYABLE(GifDecoder);

 public:
  struct Params : public DecodeParams {
    static Params Default();
  };

  GifDecoder(Params params, std::unique_ptr<io::BufReader> source);
  ~GifDecoder() override;

  // ImageDecoder implementation:
  uint32_t GetWidth() const override;
  uint32_t GetHeight() const override;
  uint64_t GetSize() const override;
  ImageType GetImageType() const override;
  ColorScheme GetColorScheme() const override;
  bool IsProgressive() const override;
  bool IsImageInfoComplete() const override;
  size_t GetFrameCount() const override;
  bool IsMultiFrame() const override;
  uint32_t GetEstimatedQuality() const override;
  bool IsFrameCompleteAtIndex(size_t index) const override;
  ImageFrame* GetFrameAtIndex(size_t index) override;
  ImageMetadata* GetMetadata() override;
  bool IsAllMetadataComplete() const override;
  bool IsAllFramesComplete() const override;
  bool IsImageComplete() const override;
  Result Decode() override;
  Result DecodeImageInfo() override;
  bool HasError() const override;

 private:
  class Impl;

  void Fail(Result error);
  Result ProcessDecodeResult(bool result);

  io::BufReader* source() { return source_.get(); }

  void set_size(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
  }

  void set_is_progressive(bool value) { is_progressive_ = value; }

  void set_color_space(ColorScheme color_scheme) {
    color_scheme_ = color_scheme;
  }

  std::vector<std::unique_ptr<ImageFrame>> image_frames_;
  ImageMetadata metadata_;
  std::unique_ptr<io::BufReader> source_;
  std::unique_ptr<Impl> impl_;
  ColorScheme color_scheme_ = ColorScheme::kUnknown;
  uint32_t width_ = 0;
  uint32_t height_ = 0;
  bool is_progressive_ = false;
  Result decode_error_ = Result::Ok();
  Params params_;
};

}  // namespace image

#endif  // IMAGE_CODECS_GIF_DECODER_H_
