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

#ifndef SQUIM_OS_FILE_H_
#define SQUIM_OS_FILE_H_

#include <string>
#include <vector>

#include "squim/io/closer.h"
#include "squim/io/flusher.h"
#include "squim/io/reader.h"
#include "squim/io/seeker.h"
#include "squim/io/writer.h"
#include "squim/os/file_info.h"
#include "squim/os/file_mode.h"
#include "squim/os/fs_result.h"

namespace os {

struct FileInfo;

typedef int FD;

class File : public io::Closer,
             public io::Flusher,
             public io::Reader,
             public io::ReaderAt,
             public io::Seeker,
             public io::Writer,
             public io::WriterAt {
 public:
  virtual FsResult Chdir() = 0;
  virtual FsResult Chmod(FileMode mode) = 0;
  virtual FsResult Chown(int uid, int gid) = 0;
  virtual FD Fd() const = 0;
  virtual const std::string& Name() = 0;
  virtual FsResult Readdir(std::vector<FileInfo>* files) = 0;
  virtual FsResult Readdirnames(std::vector<std::string>* names) = 0;
  virtual FsResult Stat(FileInfo* info) = 0;
  virtual FsResult Sync() = 0;
  virtual FsResult Truncate(size_t n) = 0;
  virtual FsResult FClose() = 0;
  virtual FsResult FRead(io::Chunk* chunk) = 0;
  virtual FsResult FReadAt(io::Chunk* chunk, size_t offset) = 0;
  virtual FsResult FSeek(size_t offset, io::Seeker::Whence whence) = 0;
  virtual FsResult FWrite(io::Chunk* chunk) = 0;
  virtual FsResult FWriteAt(io::Chunk* chunk, size_t offset) = 0;

  // io::* implementations:
  io::IoResult Close() override;
  io::IoResult Flush() override;
  io::IoResult Read(io::Chunk* chunk) override;
  io::IoResult ReadAt(io::Chunk* chunk, size_t offset) override;
  io::IoResult Seek(size_t offset, io::Seeker::Whence whence) override;
  io::IoResult Write(io::Chunk* chunk) override;
  io::IoResult WriteAt(io::Chunk* chunk, size_t offset) override;

  virtual ~File() {}
};

}  // namespace os

#endif  // SQUIM_OS_FILE_H_
