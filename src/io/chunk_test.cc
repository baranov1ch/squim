#include "io/chunk.h"

#include <memory>

#include "gtest/gtest.h"

namespace io {

TEST(StringChunkTest, CorrectAssignment) {
  std::unique_ptr<Chunk> chunk(new StringChunk("test"));
  EXPECT_EQ(4u, chunk->size());
  EXPECT_EQ("test", chunk->ToString());
}

TEST(RawChunkTest, CorrectAssignment) {
  const char kData[] = "test";
  std::unique_ptr<uint8_t[]> data(new uint8_t[4]);
  memcpy(data.get(), kData, 4);
  std::unique_ptr<Chunk> chunk(new RawChunk(std::move(data), 4));
  EXPECT_EQ(4u, chunk->size());
  EXPECT_EQ("test", chunk->ToString());
}

}  // namespace io
