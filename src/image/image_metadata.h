#ifndef IMAGE_IMAGE_METADATA_H_
#define IMAGE_IMAGE_METADATA_H_

#include <map>

#include "io/chunk.h"

namespace image {

class ImageMetadata {
 public:
  // Supported types of image metadata.
  enum class Type { kICC, kEXIF, kXMP };

  class Holder {
   public:
    void AddChunk(io::ChunkPtr chunk);
    void Freeze();

    const io::ChunkList& data() const { return data_; }
    bool frozen() const { return frozen_; }

   private:
    io::ChunkList data_;
    bool frozen_ = false;
  };

  using Data = std::map<Type, Holder>;

  const Data& data() const { return data_; }
  bool IsCompleted(Type type) const;
  bool IsAllCompleted() const;
  const io::ChunkList& Get(Type type) const;

  void Append(Type type, io::ChunkPtr data);
  void Freeze(Type type);
  void FreezeAll();

 private:
  Data data_;
};

}  // namespace image

#endif  // IMAGE_IMAGE_METADATA_H_
