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

#include "os/file_system.h"

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "os/file_system_posix.h"

namespace os {

std::unique_ptr<FileSystem> FileSystem::CreateDefault() {
  return base::make_unique<FileSystemPosix>();
}

FsResult FileSystem::Open(const std::string& path,
                          std::unique_ptr<File>* file) {
  return OpenFile(path, kReadOnly, FileMode(0), file);
}

FsResult FileSystem::Create(const std::string& path,
                            std::unique_ptr<File>* file) {
  return OpenFile(path, kReadWrite | kCreate | kTruncate, FileMode(0666), file);
}

FsResult FileSystem::RemoveAll(const std::string& path) {
  NOTIMPLEMENTED();
  return FsResult::Ok();
}

FsResult FileSystem::MkDirP(const std::string& path, FileMode permission_bits) {
  NOTIMPLEMENTED();
  return FsResult::Ok();
}

}  // namespace os
