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

#include "image/codecs/gif/lzw_writer.h"

#include "base/logging.h"
#include "io/chunk.h"

namespace image {

LZWWriter::CodeWriter::CodeWriter() {}

void LZWWriter::CodeWriter::Init(size_t data_size, size_t output_chunk_size, std::function<bool(uint8_t*, size_t)> output_cb) {
  output_chunk_size_ = output_chunk_size;
  output_.reserve(output_chunk_size);
  output_it_ = output_.begin();
  output_cb_ = output_cb;
  ResetCodesize(data_size);
}

bool LZWWriter::CodeWriter::Write(uint16_t code) {
  DCHECK_GT(8, bits_in_buffer_);
  buffer_ |= static_cast<uint32_t>(code) << bits_in_buffer_;
  bits_in_buffer_ += codesize_;
  while (bits_in_buffer_ >= 8) {
    auto byte = static_cast<uint8_t>(buffer_ & 0xFF);
    bits_in_buffer_ -= 8;
    buffer_ >>= 8;
    *output_it_++ = byte;
    if (output_.size() == output_chunk_size_) {
      if (!output_cb_(&output_[0], output_chunk_size_))
        return false;
      output_it_ = output_.begin();
    }
  }
  return true;
}

bool LZWWriter::CodeWriter::Finish() {
  DCHECK_GT(8, bits_in_buffer_);
  if (bits_in_buffer_ > 0) {
    size_t reminder = 8 - bits_in_buffer_;
    auto byte = static_cast<uint8_t>((buffer_ & 0xFF) << reminder);
    *output_it_++ = byte;
  }
  return output_cb_(&output_[0], output_it_ - output_.begin());
}

bool LZWWriter::CodeWriter::CodeFitsCurrentCodesize(size_t code) const {
  return !(code & codemask_);
}

void LZWWriter::CodeWriter::IncreaseCodesize() {
  ResetCodesize(codesize_ + 1);
}

void LZWWriter::CodeWriter::ResetCodesize(size_t data_size) {
  codesize_ = data_size + 1;
  codemask_ = (1 << codesize_) - 1;
}

LZWWriter::LZWWriter() {}

LZWWriter::~LZWWriter() {}

bool LZWWriter::Init(size_t data_size, size_t output_chunk_size, std::function<bool(uint8_t*, size_t)> output_cb) {
  if (data_size > kMaxCodeSize)
    return false;

  data_size_ = data_size;
  clear_code_ = 1 << data_size_;
  eoi_ = clear_code_ + 1;
  next_code_ = eoi_ + 1;

  out_.Init(data_size_, output_chunk_size, output_cb);
  for (size_t i = 0; i < clear_code_; ++i) {
    IndexBuffer buf;
    buf.push_back(static_cast<uint8_t>(i));
    code_table_[buf] = i;
  }

  return true;
}

io::IoResult LZWWriter::Write(io::Chunk* chunk) {
  return Write(chunk->data(), chunk->size());
}

io::IoResult LZWWriter::Write(uint8_t* data, size_t len) {
  uint8_t* end = data + len;
  if (!started_) {
    started_ = true;
    if (!out_.Write(clear_code_))
      return io::IoResult::Error();

    index_buffer_.push_back(*data++);
  }

  DCHECK_GT(kMaxDictionarySize, code_table_.size());

  while (data < end) {
    auto index = *data++;
    LOG(INFO) << "index=" << static_cast<size_t>(index)  << " index_buffer " << index_buffer_.size() << " code_t " << code_table_.size();
    auto it = code_table_.find(index_buffer_);
    DCHECK(it != code_table_.end());
    last_code_in_index_buffer_ = it->second;
    index_buffer_.push_back(index);
    if (code_table_.find(index_buffer_) == code_table_.end()) {
      IndexBuffer copy(index_buffer_);
      code_table_.insert({copy, next_code_++});
      code_table_.find(index_buffer_);
      code_table_.find(index_buffer_);
      code_table_.find(index_buffer_);
      index_buffer_.clear();

      if (!out_.Write(last_code_in_index_buffer_))
        return io::IoResult::Error();

      if (!out_.CodeFitsCurrentCodesize(next_code_)) {
        out_.IncreaseCodesize();
      }
      if (code_table_.size() == kMaxDictionarySize) {
        LOG(INFO) << "Clearing...";
        if (!out_.Write(clear_code_))
          return io::IoResult::Error();

        out_.ResetCodesize(data_size_);
        code_table_.clear();
      }
      if (code_table_.size() == 96) {
        LOG(INFO) << "KEKEKE1 ";
        code_table_.find(index_buffer_);
      }
      index_buffer_.push_back(index);
      if (code_table_.size() == 96) {
        LOG(INFO) << "KEKEKE2 ";
        code_table_.find(index_buffer_);
      }
    }
  }

  return io::IoResult::Write(len);
}

io::IoResult LZWWriter::Finish() {
  if (!out_.Write(eoi_) || !out_.Finish())
    return io::IoResult::Error();

  return io::IoResult::Write(0);
}

}  // namespace image
