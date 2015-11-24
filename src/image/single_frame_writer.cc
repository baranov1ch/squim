#include "image/single_frame_writer.h"

#include "image/image_encoder.h"

namespace image {

SingleFrameWriter::SingleFrameWriter(std::unique_ptr<ImageEncoder> encoder)
    : encoder_(std::move(encoder)) {}

SingleFrameWriter::~SingleFrameWriter() {}

void SingleFrameWriter::SetMetadata(const ImageMetadata* metadata) {
  encoder_->SetMetadata(metadata);
}

Result SingleFrameWriter::WriteFrame(ImageFrame* frame) {
  if (frame_written_)
    return Result::Error(
        Result::Code::kFailed,
        "Attempt to write multiple frames using SingleFrameWriter");

  frame_written_ = true;
  return encoder_->EncodeFrame(frame, true);
}

Result SingleFrameWriter::FinishWrite() {
  return encoder_->FinishWrite();
}

}  // namespace image
