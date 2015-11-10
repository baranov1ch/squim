#ifndef IMAGE_IMAGE_READER_H_
#define IMAGE_IMAGE_READER_H_

#include <memory>

namespace image {

class ImageReader {
  MAKE_NONCOPYABLE(ImageReader);
 public:
  ImageReader(std::unique_ptr<ImageDecoder> decoder);
  ~ImageReader();

  Result GetImageInfo(ImageInfo** info);

  bool HasMoreFrames() const;
  Result GetNextFrame(ImageFrame** frame);

  Result GetMetadata(ImageMetadata** metadata);

 private:
  size_t current_frame_idx_;
  std::unique_ptr<ImageDecoder> decoder_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_READER_H_
