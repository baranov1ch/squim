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

#ifndef IOUTIL_CHUNK_WRITER_H_
#define IOUTIL_CHUNK_WRITER_H_

#include "io/chunk.h"
#include "io/writer.h"

namespace ioutil {

class ChunkListWriter : public io::Writer {
 public:
  explicit ChunkListWriter(io::ChunkList* out);

  io::IoResult Write(io::Chunk* chunk) override;

 private:
  io::ChunkList* out_;
};

}  // namespace ioutil

#endif  // IOUTIL_CHUNK_WRITER_H_
