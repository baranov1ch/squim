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

#include "squim/io/buf_writer.h"

#include <algorithm>
#include <cstring>

#include "squim/base/logging.h"

namespace io {

BufWriter::BufWriter(size_t buf_size, std::unique_ptr<Writer> underlying)
    : buf_size_(buf_size), underlying_(std::move(underlying)) {}

BufWriter::~BufWriter() {}

IoResult BufWriter::Write(Chunk* chunk) {
  if (flushing_)
    return IoResult::Pending();

  if (!buffer_)
    buffer_ = Chunk::New(buf_size_);

  size_t nwrite = 0;
  for (;;) {
    auto slice = chunk->Slice(nwrite);
    auto effective_len = std::min(available(), slice->size());
    auto to_copy = slice->Slice(0, effective_len);

    std::memcpy(buffer_->data() + offset_, to_copy->data(), effective_len);
    offset_ += effective_len;
    nwrite += effective_len;

    if (available() == 0) {
      auto result = Flush();
      if (result.error())
        return result;

      if (result.pending())
        return IoResult::Write(nwrite);

      DCHECK_EQ(0, start_);
      DCHECK_EQ(0, offset_);
    }

    if (nwrite == chunk->size())
      return IoResult::Write(nwrite);
  }

  NOTREACHED();
  return IoResult::Error();
}

IoResult BufWriter::Flush() {
  DCHECK(underlying_);

  if (!buffer_)
    return IoResult::Write(0);

  flushing_ = true;

  size_t nwrite = 0;
  for (;;) {
    auto to_write = buffer_->Slice(start_, offset_ - start_);
    auto result = underlying_->Write(to_write.get());
    if (result.pending()) {
      return result;
    }

    if (result.error()) {
      flushing_ = false;
      return result;
    }

    start_ += result.n();
    nwrite += result.n();
    if (start_ == offset_) {
      // Success.
      start_ = 0;
      offset_ = 0;
      flushing_ = false;
      return IoResult::Write(nwrite);
    }
  }

  NOTREACHED();
  return IoResult::Error();
}

ChunkPtr BufWriter::ReleaseBuffer() {
  auto ret = Chunk::Wrap(std::move(buffer_), start_, offset_ - start_);
  start_ = 0;
  offset_ = 0;
  flushing_ = false;
  return ret;
}

}  // namespace io
