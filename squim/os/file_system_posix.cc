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

#include "squim/os/file_system_posix.h"

#include "squim/base/logging.h"
#include "squim/os/file_posix.h"
#include "squim/os/posix_util.h"

namespace os {

FileSystemPosix::FileSystemPosix() {}

FileSystemPosix::~FileSystemPosix() {}

FsResult FileSystemPosix::OpenFile(const std::string& path,
                                   int flags,
                                   FileMode permission_bits,
                                   std::unique_ptr<File>* file) {
  auto int_flags = static_cast<int>(flags) | O_CLOEXEC;
  int fd = open(path.c_str(), int_flags, FileModeToUnixModeT(permission_bits));
  if (fd < 0)
    return FsResult::Error(OsError::Error(errno), "open", path);

  file->reset(new FilePosix(fd, path));
  return FsResult::Ok();
}

FsResult FileSystemPosix::CreateTempFile(const std::string& prefix,
                                         std::unique_ptr<File>* file) {
  NOTIMPLEMENTED();
  return FsResult::Ok();
}

FsResult FileSystemPosix::MkDir(const std::string& path,
                                FileMode permission_bits) {
  if (mkdir(path.c_str(), FileModeToUnixModeT(permission_bits)) != 0)
    return FsResult::Error(OsError::Error(errno), "mkdir", path);

  return FsResult::Ok();
}

FsResult FileSystemPosix::Remove(const std::string& path) {
  if (unlink(path.c_str()) == 0)
    return FsResult::Ok();

  if (errno == EISDIR && rmdir(path.c_str()) != 0)
    return FsResult::Error(OsError::Error(errno), "rmdir", path);

  return FsResult::Error(OsError::Error(errno), "unlink", path);
}

FsResult FileSystemPosix::Rename(const std::string& old_path,
                                 const std::string& new_path) {
  if (rename(old_path.c_str(), new_path.c_str()) != 0)
    return FsResult::Error(OsError::Error(errno), "rename",
                           old_path + ", " + new_path);

  return FsResult::Ok();
}

FsResult FileSystemPosix::Symlink(const std::string& old_path,
                                  const std::string& new_path) {
  if (symlink(old_path.c_str(), new_path.c_str()) != 0)
    return FsResult::Error(OsError::Error(errno), "symlink",
                           old_path + ", " + new_path);

  return FsResult::Ok();
}

std::string FileSystemPosix::TempDir() {
  return "/tmp";
}

FsResult FileSystemPosix::Lstat(const std::string& path, FileInfo* file_info) {
  DCHECK(file_info);
  struct stat statbuf;
  if (lstat(path.c_str(), &statbuf) != 0) {
    return FsResult::Error(OsError::Error(errno), "lstat", path);
  }

  file_info->name = path;
  StatToFileInfo(&statbuf, file_info);
  return FsResult::Ok();
}

FsResult FileSystemPosix::Stat(const std::string& path, FileInfo* file_info) {
  DCHECK(file_info);
  struct stat statbuf;
  if (stat(path.c_str(), &statbuf) != 0) {
    return FsResult::Error(OsError::Error(errno), "stat", path);
  }

  file_info->name = path;
  StatToFileInfo(&statbuf, file_info);
  return FsResult::Ok();
}

}  // namespace os
