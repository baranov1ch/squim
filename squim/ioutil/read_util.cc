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

#include "squim/ioutil/read_util.h"

#include "squim/base/logging.h"
#include "squim/io/reader.h"
#include "squim/io/writer.h"
#include "squim/ioutil/chunk_writer.h"

namespace ioutil {

namespace {
const size_t kStackBufferSize = 10000;
}

io::IoResult Copy(io::Writer* dst, io::Reader* src, size_t chunk_size) {
  auto chunk = io::Chunk::New(chunk_size);
  auto read_result = io::IoResult::Read(0);
  auto write_result = io::IoResult::Write(0);
  size_t nread = 0;
  while (!read_result.eof()) {
    read_result = src->Read(chunk.get());
    DCHECK(!read_result.pending());
    if (!read_result.ok() && !read_result.eof())
      return read_result;

    auto slice = chunk->Slice(0, read_result.n());
    write_result = dst->Write(slice.get());
    DCHECK(!write_result.pending());

    if (write_result.n() < slice->size())
      return io::IoResult::Error("Short write");

    if (!write_result.ok())
      return write_result;

    nread += write_result.n();
  }
  return io::IoResult::Read(nread);
}

io::IoResult Copy(io::Writer* dst, io::Reader* src) {
  return Copy(dst, src, kStackBufferSize);
}

io::IoResult ReadFull(io::Reader* reader, io::ChunkList* out) {
  ChunkListWriter writer(out);
  return Copy(&writer, reader);
}

io::IoResult ReadFull(io::Reader* reader, io::ChunkPtr* out) {
  io::ChunkList chunks;
  ChunkListWriter writer(&chunks);
  auto result = Copy(&writer, reader);
  if (result.ok())
    *out = io::Chunk::Merge(chunks);
  return result;
}

}  // namespace ioutil
