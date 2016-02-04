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

#ifndef OS_FILE_SYSTEM_H_
#define OS_FILE_SYSTEM_H_

#include <memory>
#include <string>

#include "os/fs_result.h"
#include "os/file_mode.h"

namespace os {

class File;
struct FileInfo;

class FileSystem {
 public:
  static std::unique_ptr<FileSystem> CreateDefault();
  FsResult Open(const std::string& path, std::unique_ptr<File>* file);
  FsResult Create(const std::string& path, std::unique_ptr<File>* file);
  virtual FsResult OpenFile(const std::string& path,
                            int flags,
                            FileMode permission_bits,
                            std::unique_ptr<File>* file) = 0;
  virtual FsResult CreateTempFile(const std::string& prefix,
                                  std::unique_ptr<File>* file) = 0;
  virtual FsResult MkDir(const std::string& path, FileMode permission_bits) = 0;
  FsResult MkDirP(const std::string& path, FileMode permission_bits);
  virtual FsResult Remove(const std::string& path) = 0;
  FsResult RemoveAll(const std::string& path);
  virtual FsResult Rename(const std::string& old_path,
                          const std::string& new_path) = 0;
  virtual FsResult Symlink(const std::string& old_path,
                           const std::string& new_path) = 0;
  virtual std::string TempDir() = 0;
  virtual FsResult Lstat(const std::string& path, FileInfo* file_info) = 0;
  virtual FsResult Stat(const std::string& path, FileInfo* file_info) = 0;

#if 0  // TODO
  virtual io::Writer* Stdout() const = 0;
  virtual io::Writer* Stderr() const = 0;
  virtual io::Reader* Stdin() const = 0;
#endif

  virtual ~FileSystem() {}
};

}  // namespace os

#endif  // OS_FILE_SYSTEM_H_
