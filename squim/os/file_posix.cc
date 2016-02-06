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

#include <dirent.h>
#include <unistd.h>

#include <cstring>

#include <sys/stat.h>

#include "squim/base/logging.h"
#include "squim/base/strings/string_util.h"
#include "squim/io/chunk.h"
#include "squim/os/file_posix.h"
#include "squim/os/posix_util.h"

namespace os {

FilePosix::FilePosix(FD fd, const std::string& filename)
    : fd_(fd), filename_(filename) {}

FilePosix::~FilePosix() {
  FClose();
}

FsResult FilePosix::Chdir() {
  if (fchdir(fd_) != 0)
    return FsResult::Error(OsError::Error(errno), "fchdir", filename_);

  return FsResult::Ok();
}

FsResult FilePosix::Chmod(FileMode mode) {
  if (fchmod(fd_, FileModeToUnixModeT(mode)) != 0)
    return FsResult::Error(OsError::Error(errno), "fchmod", filename_);

  return FsResult::Ok();
}

FsResult FilePosix::Chown(int uid, int gid) {
  int result = fchown(fd_, uid, gid);
  if (result)
    return FsResult::Error(OsError::Error(errno), "fchown", filename_);

  return FsResult::Ok();
}

FD FilePosix::Fd() const {
  return fd_;
}

const std::string& FilePosix::Name() {
  return filename_;
}

FsResult FilePosix::Readdir(std::vector<FileInfo>* files) {
  DCHECK(files);

  std::vector<std::string> names;
  auto result = Readdirnames(&names);
  if (!result.ok())
    return result;

  for (const auto& name : names) {
    struct stat statbuf;
    if (lstat(name.c_str(), &statbuf) != 0) {
      if (errno == ENOENT) {
        // File disappeared between readdir and lstat.
        continue;
      } else {
        return FsResult::Error(OsError::Error(errno), "lstat", name);
      }
    }

    FileInfo file_info;
    file_info.name = name;
    StatToFileInfo(&statbuf, &file_info);

    files->push_back(file_info);
  }

  return FsResult::Ok();
}

FsResult FilePosix::Readdirnames(std::vector<std::string>* names) {
  DCHECK(names);
  DIR* dir = fdopendir(fd_);
  if (!dir)
    return FsResult::Error(OsError::Error(errno), "fdopendir", filename_);

  std::string dir_string = filename_;
  base::EnsureEndsInSlash(&dir_string);

  dirent* entry = nullptr;
  dirent buffer;
  while (readdir_r(dir, &buffer, &entry) == 0 && entry != nullptr) {
    if ((std::strcmp(entry->d_name, ".") != 0) &&
        (std::strcmp(entry->d_name, "..") != 0)) {
      names->push_back(dir_string + entry->d_name);
    }
  }

  if (closedir(dir) != 0)
    return FsResult::Error(OsError::Error(errno), "closedir", filename_);

  return FsResult::Ok();
}

FsResult FilePosix::Stat(FileInfo* info) {
  struct stat statbuf;
  if (fstat(fd_, &statbuf) != 0)
    return FsResult::Error(OsError::Error(errno), "fstat", filename_);

  FileInfo file_info;
  file_info.name = filename_;
  StatToFileInfo(&statbuf, &file_info);
  return FsResult::Ok();
}

FsResult FilePosix::Sync() {
  if (HANDLE_EINTR(fsync(fd_)) != 0)
    return FsResult::Error(OsError::Error(errno), "fsync", filename_);

  return FsResult::Ok();
}

FsResult FilePosix::Truncate(size_t n) {
  if (HANDLE_EINTR(ftruncate(fd_, n)) != 0)
    return FsResult::Error(OsError::Error(errno), "ftruncate", filename_);

  return FsResult::Ok();
}

FsResult FilePosix::FClose() {
  auto result = FsResult::Ok();

  if (close(fd_) != 0)
    result = FsResult::Error(OsError::Error(errno), "close", filename_);

  fd_ = -1;
  return result;
}

FsResult FilePosix::FRead(io::Chunk* chunk) {
  DCHECK(chunk);
  int result = HANDLE_EINTR(read(fd_, chunk->data(), chunk->size()));
  if (result < 0)
    return FsResult::Error(OsError::Error(errno), "read", filename_);

  if (result == 0)
    return chunk->size() == 0 ? FsResult::Read(0) : FsResult::Eof();

  return FsResult::Read(result);
}

FsResult FilePosix::FReadAt(io::Chunk* chunk, size_t offset) {
  DCHECK(chunk);
  int result = HANDLE_EINTR(
      pread(fd_, chunk->data(), chunk->size(), static_cast<off_t>(offset)));
  if (result < 0)
    return FsResult::Error(OsError::Error(errno), "pread", filename_);

  if (result == 0)
    return chunk->size() == 0 ? FsResult::Read(0) : FsResult::Eof();

  return FsResult::Read(result);
}

FsResult FilePosix::FSeek(size_t offset, io::Seeker::Whence whence) {
  off_t result = HANDLE_EINTR(
      lseek(fd_, static_cast<off_t>(offset), static_cast<int>(whence)));
  if (result < 0)
    return FsResult::Error(OsError::Error(errno), "lseek", filename_);

  return FsResult::Read(static_cast<size_t>(result));
}

FsResult FilePosix::FWrite(io::Chunk* chunk) {
  if (!chunk)
    return FsResult::Write(0);

  int result = HANDLE_EINTR(write(fd_, chunk->data(), chunk->size()));
  if (result < 0)
    return FsResult::Error(OsError::Error(errno), "write", filename_);

  return FsResult::Write(result);
}

FsResult FilePosix::FWriteAt(io::Chunk* chunk, size_t offset) {
  if (!chunk)
    return FsResult::Write(0);

  int result = HANDLE_EINTR(
      pwrite(fd_, chunk->data(), chunk->size(), static_cast<off_t>(offset)));
  if (result < 0)
    return FsResult::Error(OsError::Error(errno), "pwrite", filename_);

  return FsResult::Write(result);
}

}  // namespace os
