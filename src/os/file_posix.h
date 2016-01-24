/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#ifndef OS_FILE_POSIX_H_
#define OS_FILE_POSIX_H_

#include <string>

#include "os/file.h"

namespace os {

class FilePosix : public File {
 public:
  FilePosix(FD fd, const std::string& filename);
  ~FilePosix() override;

  // File implementation:
  FsResult Chdir() override;
  FsResult Chmod(FileMode mode) override;
  FsResult Chown(int uid, int gid) override;
  FD Fd() const override;
  const std::string& Name() override;
  FsResult Readdir(std::vector<FileInfo>* files) override;
  FsResult Readdirnames(std::vector<std::string>* names) override;
  FsResult Stat(FileInfo* info) override;
  FsResult Sync() override;
  FsResult Truncate(size_t n) override;
  FsResult FClose() override;
  FsResult FRead(io::Chunk* chunk) override;
  FsResult FReadAt(io::Chunk* chunk, size_t offset) override;
  FsResult FSeek(size_t offset, io::Seeker::Whence whence) override;
  FsResult FWrite(io::Chunk* chunk) override;
  FsResult FWriteAt(io::Chunk* chunk, size_t offset) override;

 private:
  FD fd_ = -1;
  std::string filename_;
};

}  // namespace os

#endif  // OS_FILE_POSIX_H_
