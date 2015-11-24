#ifndef IMAGE_DECODING_READER_H_
#define IMAGE_DECODING_READER_H_

#include <memory>

#include "image/image_info.h"
#include "image/image_reader.h"

namespace image {

class ImageDecoder;

// Reader that simply wraps a decoder to read image from it.
class DecodingReader : public ImageReader {
 public:
  DecodingReader(std::unique_ptr<ImageDecoder> decoder);
  ~DecodingReader() override;

  // ImageReader implementation:
  bool HasMoreFrames() const override;
  const ImageMetadata* GetMetadata() const override;
  size_t GetNumberOfFramesRead() const override;
  Result GetImageInfo(const ImageInfo** info) override;
  Result GetNextFrame(ImageFrame** frame) override;
  Result GetFrameAtIndex(size_t index, ImageFrame** frame) override;
  Result ReadTillTheEnd() override;

 private:
  Result AdvanceDecode(bool header_only);

  // Number of frames already read from the decoder.
  size_t num_frames_read_ = 0;

  // True if decoder has completed image header.
  bool image_info_read_ = false;
  ImageInfo image_info_;
  std::unique_ptr<ImageDecoder> decoder_;
};

}  // namespace image

#endif  // IMAGE_DECODING_READER_H_
