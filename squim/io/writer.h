/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#ifndef SQUIM_IO_WRITER_H_
#define SQUIM_IO_WRITER_H_

#include "squim/io/chunk.h"
#include "squim/io/io_result.h"

namespace io {

class Writer {
 public:
  virtual IoResult Write(Chunk* chunk) = 0;

  virtual ~Writer() {}
};

class WriterAt {
 public:
  virtual IoResult WriteAt(Chunk* chunk, size_t offset) = 0;

  virtual ~WriterAt() {}
};

class VectorWriter {
 public:
  virtual IoResult WriteV(ChunkList chunks) = 0;

  virtual ~VectorWriter() {}
};

class DevNull : public Writer, public VectorWriter {
 public:
  IoResult Write(Chunk* chunk) override;
  IoResult WriteV(ChunkList chunks) override;
};

}  // namespace io

#endif  // SQUIM_IO_WRITER_H_
