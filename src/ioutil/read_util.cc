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

#include "ioutil/read_util.h"

#include "base/logging.h"
#include "io/reader.h"
#include "io/writer.h"
#include "ioutil/chunk_writer.h"

namespace ioutil {

namespace {
const size_t kStackBufferSize = 10000;
}

io::IoResult Copy(io::Writer* dst, io::Reader* src) {
  auto chunk = io::Chunk::New(kStackBufferSize);
  auto io_result = io::IoResult::Read(0);
  size_t nread = 0;
  while (io_result.ok() && !io_result.eof()) {
    io_result = src->Read(chunk.get());
    DCHECK(!io_result.pending());
    if (!io_result.ok())
      break;

    auto slice = chunk->Slice(0, io_result.n());
    io_result = dst->Write(slice.get());
    DCHECK(!io_result.pending());

    if (io_result.n() < slice->size())
      return io::IoResult::Error("Short write");

    if (!io_result.ok())
      break;

    nread += io_result.n();
  }

  if (nread > 0)
    return io::IoResult::Read(nread);

  return io_result;
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
