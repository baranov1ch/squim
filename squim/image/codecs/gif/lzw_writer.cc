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

#include "squim/image/codecs/gif/lzw_writer.h"

#include <algorithm>
#include <sstream>

#include <netinet/in.h>

#include "squim/base/logging.h"
#include "squim/io/chunk.h"

namespace image {

LZWWriter::CodeStream::CodeStream() {}

void LZWWriter::CodeStream::Init(
    size_t data_size,
    size_t output_chunk_size,
    std::function<bool(uint8_t*, size_t)> output_cb) {
  output_chunk_size_ = output_chunk_size;
  output_.resize(output_chunk_size);
  output_it_ = output_.begin();
  output_cb_ = output_cb;
  ResetCodesize(data_size);
}

bool LZWWriter::CodeStream::Write(uint16_t code) {
  DCHECK_GT(8, bits_in_buffer_);
  buffer_ |= static_cast<uint32_t>(code) << bits_in_buffer_;
  bits_in_buffer_ += codesize_;
  while (bits_in_buffer_ >= 8) {
    auto byte = static_cast<uint8_t>(buffer_ & 0xFF);
    bits_in_buffer_ -= 8;
    buffer_ >>= 8;
    *output_it_++ = byte;
    if (static_cast<size_t>(output_it_ - output_.begin()) ==
        output_chunk_size_) {
      if (!output_cb_(&output_[0], output_chunk_size_))
        return false;
      output_it_ = output_.begin();
    }
  }
  return true;
}

bool LZWWriter::CodeStream::Finish() {
  DCHECK_GT(8, bits_in_buffer_);
  if (bits_in_buffer_ > 0) {
    auto byte = static_cast<uint8_t>((buffer_ & 0xFF));
    *output_it_++ = byte;
  }
  return output_cb_(&output_[0], output_it_ - output_.begin());
}

bool LZWWriter::CodeStream::CodeFitsCurrentCodesize(size_t code) const {
  return code >> codesize_ == 0;
}

void LZWWriter::CodeStream::IncreaseCodesize() {
  ResetCodesize(codesize_);
}

void LZWWriter::CodeStream::ResetCodesize(size_t data_size) {
  codesize_ = data_size + 1;
}

LZWWriter::LZWWriter() {}

LZWWriter::~LZWWriter() {}

bool LZWWriter::Init(size_t data_size,
                     size_t output_chunk_size,
                     std::function<bool(uint8_t*, size_t)> output_cb) {
  if (data_size > kMaxCodeSize)
    return false;

  data_size_ = data_size;
  clear_code_ = 1 << data_size_;
  eoi_ = clear_code_ + 1;
  code_table_.reserve(kMaxDictionarySize);

  code_stream_.Init(data_size_, output_chunk_size, output_cb);
  Clear();

  return true;
}

void LZWWriter::Clear() {
  code_table_.clear();
  next_code_ = eoi_ + 1;
  code_stream_.ResetCodesize(data_size_);

  // Fill trivial codes, i.e. colors.
  for (size_t i = 0; i < clear_code_; ++i) {
    IndexBuffer buf;
    buf.push_back(i);
    code_table_.insert({buf, i});
  }
}

io::IoResult LZWWriter::Write(io::Chunk* chunk) {
  return Write(chunk->data(), chunk->size());
}

io::IoResult LZWWriter::Write(const uint8_t* data, size_t len) {
  const uint8_t* end = data + len;
  if (!started_) {
    started_ = true;
    if (!code_stream_.Write(clear_code_))
      return io::IoResult::Error();

    index_buffer_.push_back(*data++);
    auto it = code_table_.find(index_buffer_);
    DCHECK(it != code_table_.end());
    last_code_in_index_buffer_ = it->second;
  }

  DCHECK_GT(kMaxDictionarySize, code_table_.size());
  while (data < end) {
    auto index = *data++;
    index_buffer_.push_back(index);
    if (code_table_.find(index_buffer_) == code_table_.end()) {
      if (!code_stream_.Write(last_code_in_index_buffer_))
        return io::IoResult::Error();

      if (!code_stream_.CodeFitsCurrentCodesize(next_code_))
        code_stream_.IncreaseCodesize();

      code_table_.insert({std::move(index_buffer_), next_code_++});
      index_buffer_.clear();
      if (next_code_ == kMaxDictionarySize) {
        if (!code_stream_.Write(clear_code_))
          return io::IoResult::Error();
        Clear();
      }
      index_buffer_.push_back(index);
    }

    auto it = code_table_.find(index_buffer_);
    DCHECK(it != code_table_.end());
    last_code_in_index_buffer_ = it->second;
  }
  if (!code_stream_.Write(last_code_in_index_buffer_))
    return io::IoResult::Error();

  return io::IoResult::Write(len);
}

io::IoResult LZWWriter::Finish() {
  if (!code_stream_.Write(eoi_) || !code_stream_.Finish())
    return io::IoResult::Error();

  return io::IoResult::Write(0);
}

}  // namespace image
