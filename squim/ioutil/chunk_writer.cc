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

#include "squim/ioutil/chunk_writer.h"

#include "squim/base/logging.h"
#include "squim/io/chunk.h"

namespace ioutil {

ChunkListWriter::ChunkListWriter(io::ChunkList* out) : out_(out) {
  DCHECK(out_);
}

io::IoResult ChunkListWriter::Write(io::Chunk* chunk) {
  out_->push_back(chunk->Clone());
  return io::IoResult::Write(chunk->size());
}

}  // namespace ioutil
