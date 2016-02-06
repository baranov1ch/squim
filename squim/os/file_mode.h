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

#ifndef SQUIM_OS_FILE_MODE_H_
#define SQUIM_OS_FILE_MODE_H_

#include <cstdint>
#include <fcntl.h>

namespace os {

enum FileFlags : int {
  kReadOnly = O_RDONLY,
  kWriteOnly = O_WRONLY,
  kReadWrite = O_RDWR,
  kAppend = O_APPEND,
  kCreate = O_CREAT,
  kExcl = O_EXCL,
  kSync = O_SYNC,
  kTruncate = O_TRUNC,
};

class FileMode {
 public:
  enum {
    kDir = 1 << 31,
    kAppend = 1 << 30,
    kExclusive = 1 << 29,
    kTemporary = 1 << 28,
    kSymlink = 1 << 27,
    kDevice = 1 << 26,
    kNamedPipe = 1 << 25,
    kSocket = 1 << 24,
    kSetuid = 1 << 23,
    kSetgid = 1 << 22,
    kCharDevice = 1 << 21,
    kSticky = 1 << 20,
    kType = kDir | kSymlink | kNamedPipe | kSocket | kDevice,
    kPerm = 0777,
  };

  FileMode() {}
  explicit FileMode(uint32_t os_mode) : os_mode_(os_mode) {}

  bool is_dir() const { return (os_mode_ & kDir) != 0; }
  bool is_regular() const { return (os_mode_ & kType) == 0; }
  FileMode perm() const { return FileMode(os_mode_ & kPerm); }
  uint32_t mode() const { return os_mode_; }

 private:
  uint32_t os_mode_ = 0;
};

}  // namespace os

#endif  // SQUIM_OS_FILE_MODE_H_
