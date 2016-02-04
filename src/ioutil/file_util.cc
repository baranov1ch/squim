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

#include "ioutil/file_util.h"

#include "base/logging.h"
#include "ioutil/chunk_reader.h"
#include "ioutil/chunk_writer.h"
#include "ioutil/read_util.h"
#include "ioutil/string_reader.h"
#include "ioutil/string_writer.h"
#include "os/file.h"
#include "os/file_system.h"

namespace ioutil {

os::FsResult ReadDir(const std::string& path,
                     std::vector<os::FileInfo>* contents) {
  auto fs = os::FileSystem::CreateDefault();
  std::unique_ptr<os::File> dir;
  auto result = fs->Open(path, &dir);
  if (!result.ok())
    return result;

  return dir->Readdir(contents);
}

io::IoResult ReadFile(const std::string& path, io::Writer* writer) {
  auto fs = os::FileSystem::CreateDefault();
  std::unique_ptr<os::File> file;
  auto result = fs->Open(path, &file);
  if (!result.ok())
    return result.ToIoResult();

  return Copy(writer, file.get());
}

io::IoResult ReadFile(const std::string& path, io::ChunkPtr* chunk) {
  // TODO: make some space reservation after Stat.
  io::ChunkList chunks;
  ChunkListWriter writer(&chunks);
  auto result = ReadFile(path, &writer);
  *chunk = io::Chunk::Merge(chunks);
  return result;
}

io::IoResult ReadFile(const std::string& path, io::ChunkList* chunks) {
  ChunkListWriter writer(chunks);
  return ReadFile(path, &writer);
}

io::IoResult ReadFile(const std::string& path, std::string* contents) {
  StringWriter writer(contents);
  return ReadFile(path, &writer);
}

io::IoResult WriteFile(const std::string& path,
                       io::Reader* reader,
                       os::FileMode perm) {
  auto fs = os::FileSystem::CreateDefault();
  std::unique_ptr<os::File> file;
  auto result = fs->OpenFile(path, os::kReadWrite | os::kCreate | os::kTruncate,
                             perm, &file);
  if (!result.ok())
    return result.ToIoResult();

  // TODO: inefficient stuff!
  return Copy(file.get(), reader);
}

io::IoResult WriteFile(const std::string& path,
                       const io::Chunk* chunk,
                       os::FileMode perm) {
  ChunkReader reader(chunk);
  return WriteFile(path, &reader, perm);
}

io::IoResult WriteFile(const std::string& path,
                       const io::ChunkList* chunks,
                       os::FileMode perm) {
  ChunkListReader reader(chunks);
  return WriteFile(path, &reader, perm);
}

io::IoResult WriteFile(const std::string& path,
                       const std::string& contents,
                       os::FileMode perm) {
  StringReader reader(contents);
  return WriteFile(path, &reader, perm);
}

}  // namespace ioutil
