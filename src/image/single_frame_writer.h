#ifndef IMAGE_SINGLE_FRAME_WRITER_H_
#define IMAGE_SINGLE_FRAME_WRITER_H_

#include <memory>

#include "base/make_noncopyable.h"
#include "image/image_writer.h"

namespace image {

class ImageEncoder;

class SingleFrameWriter : public ImageWriter {
  MAKE_NONCOPYABLE(SingleFrameWriter);

 public:
  SingleFrameWriter(std::unique_ptr<ImageEncoder> encoder);
  ~SingleFrameWriter() override;

  void SetMetadata(const ImageMetadata* metadata) override;
  Result WriteFrame(ImageFrame* frame) override;
  Result FinishWrite() override;

 private:
  std::unique_ptr<ImageEncoder> encoder_;
  bool frame_written_ = false;
};

}  // namespace image

#endif  // IMAGE_SINGLE_FRAME_WRITER_H_
