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

#ifndef IOUTIL_FILE_UTIL_H_
#define IOUTIL_FILE_UTIL_H_

#include <string>
#include <vector>

#include "io/chunk.h"
#include "io/io_result.h"
#include "os/file_info.h"
#include "os/file_mode.h"
#include "os/fs_result.h"

namespace io {
class Reader;
class Writer;
}

namespace ioutil {

os::FsResult ReadDir(const std::string& path,
                     std::vector<os::FileInfo>* contents);

io::IoResult ReadFile(const std::string& path, io::Writer* writer);

io::IoResult ReadFile(const std::string& path, io::ChunkPtr* chunk);

io::IoResult ReadFile(const std::string& path, io::ChunkList* chunks);

io::IoResult ReadFile(const std::string& path, std::string* contents);

io::IoResult WriteFile(const std::string& path,
                       io::Reader* reader,
                       os::FileMode perm);

io::IoResult WriteFile(const std::string& path,
                       const io::Chunk* chunk,
                       os::FileMode perm);

io::IoResult WriteFile(const std::string& path,
                       const io::ChunkList* chunks,
                       os::FileMode perm);

io::IoResult WriteFile(const std::string& path,
                       const std::string& contents,
                       os::FileMode perm);

}  // namespace ioutil

#endif  // IOUTIL_FILE_UTIL_H_
