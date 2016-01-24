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

#include "os/file.h"

namespace os {

io::IoResult File::Close() {
  return FClose().ToIoResult();
}

io::IoResult File::Flush() {
  return Sync().ToIoResult();
}

io::IoResult File::Read(io::Chunk* chunk) {
  return FRead(chunk).ToIoResult();
}

io::IoResult File::ReadAt(io::Chunk* chunk, size_t offset) {
  return FReadAt(chunk, offset).ToIoResult();
}

io::IoResult File::Seek(size_t offset, io::Seeker::Whence whence) {
  return FSeek(offset, whence).ToIoResult();
}

io::IoResult File::Write(io::Chunk* chunk) {
  return FWrite(chunk).ToIoResult();
}

io::IoResult File::WriteAt(io::Chunk* chunk, size_t offset) {
  return FWriteAt(chunk, offset).ToIoResult();
}

}  // namespace os
