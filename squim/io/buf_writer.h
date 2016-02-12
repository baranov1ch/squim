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

#ifndef SQUIM_IO_BUF_WRITER_H_
#define SQUIM_IO_BUF_WRITER_H_

#include <memory>

#include "squim/io/chunk.h"
#include "squim/io/flusher.h"
#include "squim/io/writer.h"

namespace io {

class BufWriter : public Writer, public Flusher {
 public:
  BufWriter(size_t buf_size, std::unique_ptr<Writer> underlying);
  ~BufWriter() override;

  // Writer implementation:
  IoResult Write(Chunk* chunk) override;

  // Flusher implementation:
  IoResult Flush() override;

  size_t available() const { return buffer_->size() - offset_; }
  size_t buffered() const { return offset_; }

  ChunkPtr ReleaseBuffer();

 private:
  size_t buf_size_;
  ChunkPtr buffer_;
  size_t start_ = 0;
  size_t offset_ = 0;
  bool flushing_ = false;
  std::unique_ptr<Writer> underlying_;
};

}  // namespace io

#endif  // SQUIM_IO_BUF_WRITER_H_
