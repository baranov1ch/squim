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

#ifndef SQUIM_OS_DIR_UTIL_H_
#define SQUIM_OS_DIR_UTIL_H_

#include <string>
#include <vector>

#include "squim/os/file_info.h"
#include "squim/os/fs_result.h"

namespace os {

class File;
class FileSystem;

FsResult ReaddirnamesRecursively(FileSystem* fs,
                                 const std::string& root,
                                 size_t max_depth,
                                 std::vector<std::string>* out);

FsResult ReaddirnamesRecursively(const std::string& root,
                                 size_t max_depth,
                                 std::vector<std::string>* out);

FsResult ReaddirRecursively(FileSystem* fs,
                            const std::string& root,
                            size_t max_depth,
                            std::vector<FileInfo>* out);

FsResult ReaddirRecursively(const std::string& root,
                            size_t max_depth,
                            std::vector<FileInfo>* out);

}  // namespace os

#endif  // SQUIM_OS_DIR_UTIL_H_
