#ifndef IMAGE_IMAGE_DECODER_H_
#define IMAGE_IMAGE_DECODER_H_

#include "image/image_constants.h"

namespace image {

class ImageFrame;
class ImageMetadata;

// Decoder interface.
class ImageDecoder {
 public:
  // Some geometry for the decoded image.
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;

  // Image size in bytes.
  virtual uint64_t GetSize() const = 0;

  virtual ImageType GetImageType() const = 0;
  virtual ColorScheme GetColorScheme() const = 0;

  // Should return true if image header (containing width, height, and stuff)
  // has been parsed and client can call any of the methods above.
  virtual bool IsImageInfoComplete() const = 0;

  // Returns number of frames *decoded so far*. 1 does not generally mean that
  // image is single frame, other may not be decoded yet
  virtual uint32_t GetFrameCount() const = 0;

  // Returns true if image could be multiframe (like gif/webp).
  virtual bool IsMultiFrame() const = 0;

  // Should return true iff frame at |index| has been successfully decoded
  // so far.
  virtual bool IsFrameCompleteAtIndex(size_t index) = 0;

  // Should return non-owned pointer to the decoded image frame. Implementations
  // may assume that |index| refers to a parsed frame - it's up to the client to
  // ensure it using IsFrameCompleteAtIndex().
  virtual ImageFrame* GetFrameAtIndex(size_t index) = 0;

  // Returns duration of the frame. If image is not animated, returns
  // |kInvalidDuration|. It's up to the client to ensure that requested frame is
  // ready using IsFrameCompleteAtIndex().
  virtual uint32_t GetFrameDurationAtIndex(size_t index) = 0;

  // Returns image metadata.
  virtual ImageMetadata* GetMetadata() = 0;

  // Should return true if all metadata has been parsed.
  virtual bool IsAllMetadataComplete() const = 0;

  // Should return true if the whole image is ready.
  virtual bool IsImageComplete() const = 0;

  // Client callback to signal the more data is ready to parse.
  virtual Result MoreDataAvailable() = 0;

  // True if the error occurred. Decoding cannot progress after that.
  virtual bool HasError() const = 0;

  virtual ~ImageDecoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_DECODER_H_
