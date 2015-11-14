#ifndef IMAGE_IMAGE_DECODER_H_
#define IMAGE_IMAGE_DECODER_H_

namespace image {

class ImageDecoder {
 public:
  virtual ColorScheme GetColorScheme() const = 0;
  virtual uint32_t GetWidth() const = 0;
  virtual uint32_t GetHeight() const = 0;
  virtual uint64_t GetSize() const = 0;

  // Returns number of frames *decoded so far*. 1 does not generally mean that
  // image is single frame, other may not be decoded yet
  virtual uint32_t GetFrameCount() const = 0;

  // Returns true if image could be multiframe (like gif/webp).
  virtual bool IsMultiFrame() const = 0;

  virtual ImageFrame* GetFrameAtIndex(size_t index) = 0;
  virtual ImageFrame* IsFrameCompleteAtIndex(size_t index) = 0;
  virtual ImageFrame* GetFrameDurationAtIndex(size_t index) = 0;

  virtual ImageMetadata* GetMetadata() = 0;

  virtual bool IsAllDataReceived() const = 0;
  virtual bool IsImageInfoComplete() const = 0;
  virtual bool IsAllMetadataComplete() const = 0;

  virtual Result MoreDataAvailable() = 0;

  virtual ~ImageDecoder() {}
};

}  // namespace image

#endif  // IMAGE_IMAGE_DECODER_H_
