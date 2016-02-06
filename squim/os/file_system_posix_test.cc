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

#include "squim/os/file_system_posix.h"

#include <memory>

#include "squim/io/chunk.h"
#include "squim/os/file.h"

#include "gtest/gtest.h"

namespace os {

class FileSystemPosixTest : public testing::Test {
 protected:
  FileSystemPosix testee_;
};

TEST_F(FileSystemPosixTest, CreateFile) {
  std::unique_ptr<File> file;
  EXPECT_TRUE(testee_.Create(testee_.TempDir() + "/" + "tempfile", &file).ok());
  EXPECT_TRUE(testee_.Remove(file->Name()).ok());
}

TEST_F(FileSystemPosixTest, ReadWrite) {
  std::unique_ptr<File> file;
  ASSERT_TRUE(testee_.Create(testee_.TempDir() + "/" + "tempfile", &file).ok());
  auto original_chunk = io::Chunk::FromString("hello, world");
  EXPECT_EQ(original_chunk->size(), file->Write(original_chunk.get()).n());
  file.reset();

  EXPECT_TRUE(testee_.Open(testee_.TempDir() + "/" + "tempfile", &file).ok());
  auto to_chunk = io::Chunk::New(200);
  EXPECT_EQ(original_chunk->size(), file->Read(to_chunk.get()).n());
  EXPECT_EQ("hello, world",
            to_chunk->Slice(0, original_chunk->size())->ToString());
}

}  // namespace os
