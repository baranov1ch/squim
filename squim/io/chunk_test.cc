/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "squim/io/chunk.h"

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
