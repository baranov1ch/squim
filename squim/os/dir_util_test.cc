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

#include "squim/os/dir_util.h"

#include <memory>

#include "squim/base/logging.h"
#include "squim/io/chunk.h"
#include "squim/os/file.h"
#include "squim/os/file_system_posix.h"

#include "gtest/gtest.h"

namespace os {

class DirUtilTest : public testing::Test {
 protected:
  void SetUp() override { fs_ = FileSystem::CreateDefault(); }

  std::unique_ptr<FileSystem> fs_;
};

TEST_F(DirUtilTest, ReadAllFilenames) {
  auto expected_dirs = std::set<std::string>{
      "squim/os/testdata/file1.txt",
      "squim/os/testdata/subdir4/file5.txt",
      "squim/os/testdata/subdir1/subdir2/file3.txt",
      "squim/os/testdata/subdir1/subdir2/subdir3/file4.txt",
      "squim/os/testdata/subdir1/file2.txt",
  };
  std::vector<std::string> dir_contents;
  ASSERT_TRUE(
      ReaddirnamesRecursively(fs_.get(), "squim/os/testdata", 10, &dir_contents)
          .ok());
  std::set<std::string> actual_dirs(dir_contents.begin(), dir_contents.end());
  EXPECT_EQ(expected_dirs, actual_dirs);
  EXPECT_EQ(expected_dirs.size(), dir_contents.size());
}

TEST_F(DirUtilTest, ReadFilenamesLimitDepth) {
  auto expected_dirs = std::set<std::string>{
      "squim/os/testdata/file1.txt", "squim/os/testdata/subdir4/file5.txt",
      "squim/os/testdata/subdir1/subdir2",
      "squim/os/testdata/subdir1/file2.txt",
  };
  std::vector<std::string> dir_contents;
  ASSERT_TRUE(
      ReaddirnamesRecursively(fs_.get(), "squim/os/testdata", 2, &dir_contents)
          .ok());
  std::set<std::string> actual_dirs(dir_contents.begin(), dir_contents.end());
  EXPECT_EQ(expected_dirs, actual_dirs);
  EXPECT_EQ(expected_dirs.size(), dir_contents.size());
}

}  // namespace os
