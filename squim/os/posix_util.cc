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

#include "squim/os/posix_util.h"

#include <chrono>

#include "squim/base/logging.h"
#include "squim/os/file_info.h"

namespace os {

mode_t FileModeToUnixModeT(FileMode file_mode) {
  mode_t mode = file_mode.perm().mode();
  if (file_mode.mode() & FileMode::kSetuid)
    mode |= S_ISUID;

  if (file_mode.mode() & FileMode::kSetgid)
    mode |= S_ISGID;

  if (file_mode.mode() & FileMode::kSticky)
    mode |= S_ISVTX;

  return mode;
}

void StatToFileInfo(struct stat* statbuf, FileInfo* file_info) {
  DCHECK(file_info);
  DCHECK(statbuf);

  file_info->size = statbuf->st_size;
  uint32_t mode = statbuf->st_mode & 0x777;

  switch (statbuf->st_mode & S_IFMT) {
    case S_IFBLK:
      mode |= FileMode::kDevice;
      break;
    case S_IFCHR:
      mode |= FileMode::kDevice | FileMode::kCharDevice;
      break;
    case S_IFDIR:
      mode |= FileMode::kDir;
      break;
    case S_IFIFO:
      mode |= FileMode::kNamedPipe;
      break;
    case S_IFLNK:
      mode |= FileMode::kSymlink;
      break;
    case S_IFSOCK:
      mode |= FileMode::kSocket;
      break;
  }

  if (statbuf->st_mode & S_ISGID)
    mode |= FileMode::kSetgid;
  if (statbuf->st_mode & S_ISUID)
    mode |= FileMode::kSetuid;
  if (statbuf->st_mode & S_ISVTX)
    mode |= FileMode::kSticky;

  file_info->mode = FileMode(mode);

  auto d = std::chrono::seconds{statbuf->st_mtim.tv_sec} +
           std::chrono::nanoseconds{statbuf->st_mtim.tv_nsec};
  file_info->mtime = std::chrono::system_clock::time_point(
      std::chrono::duration_cast<std::chrono::system_clock::duration>(d));
}

}  // namespace os
