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

#include "io/buffered_source.h"

#include <cstring>

#include "base/logging.h"
#include "base/memory/make_unique.h"

namespace io {

BufferedSource::BufferedSource() : current_chunk_(chunks_.end()) {}

BufferedSource::~BufferedSource() {}

bool BufferedSource::HaveSome() const {
  return current_chunk_ != chunks_.end();
}

void BufferedSource::AddChunk(ChunkPtr chunk) {
  if (chunks_.empty()) {
    CHECK(current_chunk_ == chunks_.end());
    CHECK_EQ(0, total_offset_);
    CHECK_EQ(0, offset_in_chunk_);
  }

  if (eof_received_ || chunk->size() == 0)
    return;

  total_size_ += chunk->size();
  chunks_.push_back(std::move(chunk));
  if (current_chunk_ == chunks_.end())
    current_chunk_ = std::prev(chunks_.end());
}

size_t BufferedSource::ReadSome(uint8_t** out) {
  if (!out)
    return 0;

  CHECK(HaveSome());

  auto& chunk = *current_chunk_;
  *out = chunk->data() + offset_in_chunk_;
  auto nread = chunk->size() - offset_in_chunk_;
  offset_in_chunk_ = 0;
  total_offset_ += nread;
  current_chunk_++;
  return nread;
}

size_t BufferedSource::ReadAtMostN(uint8_t** out, size_t desired) {
  if (!out || desired == 0)
    return 0;

  CHECK(HaveSome());

  auto& chunk = *current_chunk_;
  if (desired >= chunk->size() - offset_in_chunk_) {
    // Requested more than we have in one chunk, just read as much as we can.
    return ReadSome(out);
  } else {
    // Otherwiase mess up with intra-chunk offsets. OTOH we're certailly
    // reading |desired| bytes.
    *out = chunk->data() + offset_in_chunk_;
    offset_in_chunk_ += desired;
    total_offset_ += desired;
    return desired;
  }
}

bool BufferedSource::HaveN(size_t n) {
  return total_size_ - total_offset_ >= n;
}

bool BufferedSource::EofReached() const {
  return !HaveSome() && eof_received_;
}

void BufferedSource::SendEof() {
  eof_received_ = true;
}

size_t BufferedSource::UnreadN(size_t n) {
  if (n == 0 || size() == 0)
    return 0;

  if (n >= total_offset_) {
    // If more than overall size is requested, short-circuiting to just
    // resetting all offsets to zero and setting first chunk active.
    current_chunk_ = chunks_.begin();
    auto offset = total_offset_;
    offset_in_chunk_ = 0;
    total_offset_ = 0;
    return offset;
  } else if (n <= offset_in_chunk_ && current_chunk_ != chunks_.end()) {
    // Other corner case - requested |n| fits valid |*current_chunk_|. Messing
    // with offsets is enough.
    total_offset_ -= n;
    offset_in_chunk_ -= n;
    return n;
  } else {
    // The real stuff to do - iterate back over |chunks_|, find appropriate and
    // set offset in it.

    if (current_chunk_ == chunks_.end()) {
      CHECK_EQ(0, offset_in_chunk_);
    }

    // First skip the active chunk.
    size_t unread_bytes = offset_in_chunk_;
    total_offset_ -= offset_in_chunk_;
    n -= offset_in_chunk_;
    offset_in_chunk_ = 0;
    while (n > 0) {
      const auto& chunk = *(--current_chunk_);
      auto chunk_size = chunk->size();
      if (n >= chunk_size) {
        // |n| is still larger than chunk size, so just skip the chunk and go
        // to the next one.
        unread_bytes += chunk_size;
        total_offset_ -= chunk_size;
        CHECK_EQ(0, offset_in_chunk_);
        n -= chunk_size;
      } else {
        // The chunk we should stop at. Find appropriate offset in it.
        total_offset_ -= n;
        unread_bytes += n;
        offset_in_chunk_ = chunk_size - n;
        n -= n;
      }
      if (n > 0) {
        // Such cases (n > total_offset_) has to be handled before.
        CHECK(current_chunk_ != chunks_.begin());
      }
    }
    return unread_bytes;
  }
  return 0;
}

size_t BufferedSource::ReadN(uint8_t** out, size_t n) {
  if (!out || n == 0)
    return 0;

  CHECK(HaveN(n));

  auto& chunk = *current_chunk_;
  if (n <= chunk->size() - offset_in_chunk_) {
    // Short circuit - everything fits one chunk.
    auto nread = ReadAtMostN(out, n);
    CHECK_EQ(n, nread);
    return nread;
  }

  // Hard case: define chunks we need to merge.
  size_t stored_size = 0;
  auto start = current_chunk_;
  auto end = current_chunk_;
  do {
    stored_size += (*end)->size();
    end++;
  } while (stored_size - offset_in_chunk_ < n);

  // Allocate new chunk to fit all data from them.
  std::unique_ptr<uint8_t[]> new_chunk(new uint8_t[stored_size]);
  auto merged_chunk =
      base::make_unique<RawChunk>(std::move(new_chunk), stored_size);

  // Copy data.
  size_t offset = 0;
  uint8_t* dst = merged_chunk->data();
  for (auto it = start; it != end; ++it) {
    const auto& to_copy = *it;
    std::memcpy(dst + offset, to_copy->data(), to_copy->size());
    offset += to_copy->size();
  }

  // Now, remove old chunks from |chunks_|.
  auto where = chunks_.erase(start, end);

  // Finally, insert new one instead.
  current_chunk_ = chunks_.insert(where, std::move(merged_chunk));

  auto nread = ReadAtMostN(out, n);
  CHECK_EQ(n, nread);
  return nread;
}

size_t BufferedSource::FreeAtMostNBytes(size_t n) {
  if (current_chunk_ == chunks_.begin())
    return 0;

  auto start = chunks_.begin();
  auto end = start;
  size_t accumulated = 0;
  size_t accumulated_next = 0;
  for (auto it = start; it != current_chunk_; ++it) {
    const auto& chunk = *it;
    accumulated_next += chunk->size();
    if (accumulated_next >= n) {
      break;
    } else {
      accumulated = accumulated_next;
      end = it;
    }
  }
  ++end;

  chunks_.erase(start, end);
  total_size_ -= accumulated;
  total_offset_ -= accumulated;
  return accumulated;
}

size_t BufferedSource::FreeAsMuchAsPossible() {
  return FreeAtMostNBytes(size());
}

}  // namespace io
