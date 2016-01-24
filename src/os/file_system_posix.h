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

#ifndef OS_FILE_SYSTEM_POSIX_H_
#define OS_FILE_SYSTEM_POSIX_H_

#include "os/file_system.h"

namespace os {

class File;
struct FileInfo;

class FileSystemPosix : public FileSystem {
 public:
  FileSystemPosix();
  ~FileSystemPosix() override;

  FsResult Open(const std::string& path, std::unique_ptr<File>* file) override;
  FsResult Create(const std::string& path,
                  std::unique_ptr<File>* file) override;
  FsResult OpenFile(const std::string& path,
                    int flags,
                    FileMode permission_bits,
                    std::unique_ptr<File>* file) override;
  FsResult MkDir(const std::string& path, FileMode permission_bits) override;
  FsResult MkDirP(const std::string& path, FileMode permission_bits) override;
  FsResult Remove(const std::string& path) override;
  FsResult RemoveAll(const std::string& path) override;
  FsResult Rename(const std::string& old_path,
                  const std::string& new_path) override;
  FsResult Symlink(const std::string& old_path,
                   const std::string& new_path) override;
  std::string TempDir() override;
  FsResult Lstat(const std::string& path, FileInfo* file_info) override;
  FsResult Stat(const std::string& path, FileInfo* file_info) override;

#if 0  // TODO
  io::Writer* Stdout() override;
  io::Writer* Stderr() override;
  io::Reader* Stdin() override;
#endif
};

}  // namespace os

#endif  // OS_FILE_SYSTEM_H_
