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

#include "squim/os/dir_util.h"

#include "squim/base/logging.h"
#include "squim/os/file.h"
#include "squim/os/file_system.h"

namespace os {

namespace {

FsResult Recurse(const FileInfo& fi,
                 FileSystem* fs,
                 size_t depth,
                 size_t max_depth,
                 std::vector<FileInfo>* out) {
  if (depth >= max_depth) {
    out->push_back(fi);
    return FsResult::Ok();
  }
  std::vector<FileInfo> dir_contents;
  std::unique_ptr<File> dir;
  auto result = fs->Open(fi.name, &dir);
  if (!result.ok())
    return result;

  result = dir->Readdir(&dir_contents);
  if (!result.ok())
    return result;

  for (const auto& subdir_info : dir_contents) {
    if (subdir_info.is_dir()) {
      result = Recurse(subdir_info, fs, depth + 1, max_depth, out);
      if (!result.ok())
        return result;
    } else {
      out->push_back(subdir_info);
    }
  }
  return FsResult::Ok();
}

}  // namespace

FsResult ReaddirRecursively(const std::string& root,
                            size_t max_depth,
                            std::vector<FileInfo>* out) {
  auto fs = FileSystem::CreateDefault();
  return ReaddirRecursively(fs.get(), root, max_depth, out);
}

FsResult ReaddirRecursively(FileSystem* fs,
                            const std::string& root,
                            size_t max_depth,
                            std::vector<FileInfo>* out) {
  CHECK(out);
  out->clear();
  FileInfo root_file_info;
  auto result = fs->Stat(root, &root_file_info);
  if (!result.ok())
    return result;

  result = Recurse(root_file_info, fs, 0, max_depth, out);
  if (!result.ok())
    return result;

  return FsResult::Ok();
}

FsResult ReaddirnamesRecursively(const std::string& root,
                                 size_t max_depth,
                                 std::vector<std::string>* out) {
  auto fs = FileSystem::CreateDefault();
  return ReaddirnamesRecursively(fs.get(), root, max_depth, out);
}

FsResult ReaddirnamesRecursively(FileSystem* fs,
                                 const std::string& root,
                                 size_t max_depth,
                                 std::vector<std::string>* out) {
  CHECK(out);
  out->clear();
  std::vector<FileInfo> file_infos;
  auto result = ReaddirRecursively(fs, root, max_depth, &file_infos);
  if (!result.ok())
    return result;
  for (const auto& file_info : file_infos) {
    out->push_back(file_info.name);
  }
  return FsResult::Ok();
}

}  // namespace os
