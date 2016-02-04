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

#include "ioutil/file_util.h"

#include "base/logging.h"
#include "ioutil/chunk_reader.h"
#include "ioutil/chunk_writer.h"
#include "ioutil/string_reader.h"
#include "ioutil/string_writer.h"
#include "os/file.h"
#include "os/file_system.h"

namespace ioutil {

namespace {
const size_t kStackBufferSize = 10000;
}

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

  auto chunk = io::Chunk::New(kStackBufferSize);
  auto io_result = io::IoResult::Read(0);
  size_t nread = 0;
  while (io_result.ok() && !io_result.eof()) {
    io_result = file->Read(chunk.get());
    DCHECK(!io_result.pending());
    if (!io_result.ok())
      break;

    auto slice = chunk->Slice(0, io_result.n());
    io_result = writer->Write(slice.get());
    DCHECK(!io_result.pending());
    if (!io_result.ok())
      break;

    nread += io_result.n();
  }

  if (nread > 0)
    return io::IoResult::Read(nread);

  return io_result;
}

io::IoResult ReadFileChunk(const std::string& path, io::ChunkPtr* chunk) {
  // TODO: make some space reservation after Stat.
  io::ChunkList chunks;
  ChunkListWriter writer(&chunks);
  auto result = ReadFile(path, &writer);
  *chunk = std::move(io::Chunk::Merge(chunks));
  return result;
}

io::IoResult ReadFileChunks(const std::string& path, io::ChunkList* chunks) {
  ChunkListWriter writer(chunks);
  return ReadFile(path, &writer);
}

io::IoResult ReadFileToString(const std::string& path, std::string* contents) {
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
  auto chunk = io::Chunk::New(kStackBufferSize);
  auto io_result = io::IoResult::Read(0);
  size_t nwrite = 0;
  while (io_result.ok() && !io_result.eof()) {
    io_result = reader->Read(chunk.get());
    DCHECK(!io_result.pending());
    if (!io_result.ok())
      break;

    auto slice = chunk->Slice(0, io_result.n());
    io_result = file->Write(slice.get());
    DCHECK(!io_result.pending());
    if (!io_result.ok())
      break;

    if (io_result.n() < slice->size())
      return io::IoResult::Error("Short write");

    nwrite += io_result.n();
  }

  if (nwrite > 0)
    return io::IoResult::Write(nwrite);

  return io_result;
}

io::IoResult WriteFileFromChunk(const std::string& path,
                                io::Chunk* chunk,
                                os::FileMode perm) {
  ChunkReader reader(chunk);
  return WriteFile(path, &reader, perm);
}

io::IoResult WriteFileFromChunks(const std::string& path,
                                 const io::ChunkList& chunks,
                                 os::FileMode perm) {
  ChunkListReader reader(chunks);
  return WriteFile(path, &reader, perm);
}

io::IoResult WriteFileFromString(const std::string& path,
                                 const std::string& contents,
                                 os::FileMode perm) {
  StringReader reader(contents);
  return WriteFile(path, &reader, perm);
}

}  // namespace ioutil

#endif  // IOUTIL_FILE_UTIL_H_
