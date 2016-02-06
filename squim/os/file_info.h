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

#ifndef SQUIM_OS_FILE_INFO_H_
#define SQUIM_OS_FILE_INFO_H_

#include <chrono>
#include <cstdint>
#include <string>

#include "squim/os/file_mode.h"

namespace os {

struct FileInfo {
 public:
  std::string name;
  size_t size;
  FileMode mode;
  std::chrono::system_clock::time_point mtime;

  bool is_dir() const { return mode.is_dir(); }
};

}  // namespace os

#endif  // SQUIM_OS_FILE_INFO_H_
