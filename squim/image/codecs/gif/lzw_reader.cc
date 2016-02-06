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

#include "squim/image/codecs/gif/lzw_reader.h"

#include <algorithm>
#include <cstring>

#include "squim/base/logging.h"
#include "squim/io/chunk.h"

namespace image {

LZWReader::CodeStream::CodeStream() {}

void LZWReader::CodeStream::SetupBuffer(const uint8_t* buffer, size_t size) {
  input_ = buffer;
  input_size_ = size;
}

bool LZWReader::CodeStream::ReadNext(uint16_t* code) {
  // Simple read example:
  //
  // Here is our uint32_t |buffer_| as a 4-byte buffer:
  // [byte0][byte1][byte2][byte3]
  //
  // Here is it empty (dots mean bits):
  // [........][........][........][........]
  //
  // Now, reading the first byte (b mean non-empty bit).
  // |unread_bits_| is 0, |buffer_| is 0, so we literally just cast input byte
  // to uint32_t:
  // [........][........][........][b0 b1 b2 b3 b4 b5 b6 b7]
  // 8 bits are now in the input buffer, so we increase |unread_bits_| by 8.
  //
  // After that, say, we read 5-bit codes:
  // |codesize_| is 5, Codemask for 5-bit code is:
  // [00000000][00000000][00000000][00011111]
  // thus we use last 5 bits of the input to create new code by applying
  // |codemask_| to |buffer_|. Thus |code| is
  // [. . . . . . . .][. . . b3 b4 b5 b6 b7].
  //
  // Finally, we consume 5 bits from the input by shifting it right by
  // |codesize_|, so 3 first bits from the input byte left in |buffer_|:
  // [........][........][........][. . . . . b0 b1 b2].
  // We also decrease |unread_bits_| by 5, so now it is 3.
  //
  // If we had smaller |codesize_| we would repeat reading next code. But now,
  // only 3 bits left in |buffer_|, we need  at least 2 more bits to get next
  // code, so we go and read next byte from the |input_|.
  // What we want to get is:
  // [........][........][..... nb0 nb1 nb2][nb3 nb4 nb5 nb6 nb7 b0 b1 b2].
  // (nb0..nb7 - bits from next byte read)
  //
  // This is exactly what we do:
  // After casting |*input_:uint8_t| to uint32_t we have:
  // [........][........][........][nb0 nb1 nb2 nb3 nb4 nb5 nb6 nb7]
  //
  // Now all we need to do is shift those bits left by 3 (i.e. |unread_bits_|)
  // to get smth like this:
  // [........][........][..... nb0 nb1 nb2][nb3 nb4 nb5 nb6 nb7 . . .],
  //
  // and place it into our buffer (either by ADDing or by XORing).
  // Finally, we increase |unread_bits_| by another 8 an proceed consuming.
  // Loop until the end.
  while (unread_bits_ < codesize_ && input_size_ > 0) {
    buffer_ |= (static_cast<uint32_t>(*input_)) << unread_bits_;
    unread_bits_ += 8;
    input_++;
    input_size_--;
  }

  if (unread_bits_ < codesize_) {
    DCHECK_EQ(0, input_size_);
    return false;
  }

  *code = static_cast<uint16_t>(buffer_ & codemask_);
  buffer_ >>= codesize_;
  unread_bits_ -= codesize_;

  return true;
}

bool LZWReader::CodeStream::CodeFitsCurrentCodesize(size_t code) const {
  return code >> codesize_ == 0;
}

void LZWReader::CodeStream::IncreaseCodesize() {
  ResetCodesize(codesize_);
}

void LZWReader::CodeStream::ResetCodesize(size_t data_size) {
  codesize_ = data_size + 1;
  codemask_ = (1 << codesize_) - 1;
}

LZWReader::LZWReader() {}

LZWReader::~LZWReader() {}

bool LZWReader::Init(size_t data_size,
                     size_t output_chunk_size,
                     std::function<bool(uint8_t*, size_t)> output_cb) {
  if (data_size > kMaxCodeSize)
    return false;

  data_size_ = data_size;
  dictionary_.resize(kMaxDictionarySize);

  clear_code_ = 1 << data_size_;
  eoi_ = clear_code_ + 1;

  Clear();

  // Fill trivial codes, i.e colors.
  for (size_t i = 0; i < clear_code_; ++i)
    dictionary_[i] = {kNoCode, static_cast<uint8_t>(i)};

  const size_t kMaxBytes = kMaxDictionarySize - 1;
  output_chunk_size_ = output_chunk_size;
  output_.resize(output_chunk_size_ - 1 + kMaxBytes);
  output_cb_ = output_cb;
  output_it_ = output_.begin();
  return true;
}

void LZWReader::Clear() {
  code_stream_.ResetCodesize(data_size_);
  next_entry_idx_ = eoi_ + 1;
  prev_code_ = kNoCode;
  prev_code_first_byte_ = 0;
}

io::IoResult LZWReader::Decode(io::Chunk* chunk) {
  return Decode(chunk->data(), chunk->size());
}

io::IoResult LZWReader::Decode(const uint8_t* data, size_t size) {
  if (eoi_seen_) {
    return io::IoResult::Eof();
  }
  code_stream_.SetupBuffer(data, size);
  uint16_t code;
  while (code_stream_.ReadNext(&code)) {
    // clear code means we have to clean all our state - dictionary,
    // code size, etc.
    if (code == clear_code_) {
      Clear();
      continue;
    }

    // End of information, i.e. image. Signal to the client.
    if (code == eoi_) {
      auto left = output_it_ - output_.begin();
      if (left > 0 && !output_cb_(&output_[0], left))
        return io::IoResult::Error();

      output_it_ = output_.begin();
      eoi_seen_ = true;
      break;
    }

    if (!OutputCodeToStream(code)) {
      return io::IoResult::Error();
    }

    UpdateDictionary();

    if (!code_stream_.CodeFitsCurrentCodesize(next_entry_idx_) &&
        next_entry_idx_ < kMaxDictionarySize) {
      code_stream_.IncreaseCodesize();
    }

    prev_code_ = code;

    // Output to the client as much as we can by chunks of |output_chunk_size_|.
    auto chunk_start = output_.begin();
    for (; chunk_start + output_chunk_size_ <= output_it_;
         chunk_start += output_chunk_size_) {
      if (!output_cb_(&(*chunk_start), output_chunk_size_))
        return io::IoResult::Error();
    }

    // If something was consumed, move remaining data to the beginning of the
    // buffer.
    if (chunk_start != output_.begin()) {
      size_t to_copy = output_it_ - chunk_start;
      std::copy(chunk_start, chunk_start + to_copy, output_.begin());
      output_it_ = output_.begin() + to_copy;
    }
  }
  return io::IoResult::Read(size - code_stream_.available());
}

bool LZWReader::OutputCodeToStream(uint16_t code) {
  DCHECK(byte_sequence_.empty());

  // We start from the last byte in sequence coded by |code|, and follow prefix
  // references from the dictionary to reconstruct entire byte sequence
  // (remember that every byte string stored in LZW dictionary is some another
  // string from that dictionary + one byte and dictionary is stored as a
  // collection of prefix references + |value| byte). When we follow prefix
  // references, we get bytes in reverse order, but thanks to stack, when we pop
  // them later into output, they're back to normal order (LIFO FTW)!
  if (code == next_entry_idx_ && prev_code_ != kNoCode) {
    // New code - output previous code value + its first byte.
    byte_sequence_.push(prev_code_first_byte_);
    code = prev_code_;
  } else if (code > next_entry_idx_) {
    // This is an invalid code. The dictionary is just initialized
    // and the code is incomplete. We don't know how to handle
    // this case.
    return false;
  }
  // Otherwise it was a code already in dictionary, so just output it.

  // Follow backreferences and reconstruct the output until we hit
  // trivial code (i.e. |code| is less than |clear_code_|).
  while (code >= clear_code_) {
    auto& entry = dictionary_[code];
    byte_sequence_.push(entry.suffix);
    code = entry.prefix_code;
  }

  // Output the first byte of the sequence (which MUST correspond to trivial
  // code).
  DCHECK_LT(code, clear_code_);
  auto& entry = dictionary_[code];
  prev_code_first_byte_ = entry.suffix;
  byte_sequence_.push(entry.suffix);

  // Writing into output is as simple as popping items from stack.
  while (!byte_sequence_.empty()) {
    *output_it_++ = byte_sequence_.top();
    byte_sequence_.pop();
  }

  return true;
}

void LZWReader::UpdateDictionary() {
  // Add |prev_code_| + |prev_code_first_byte_| to the code table.
  if (next_entry_idx_ < kMaxDictionarySize && prev_code_ != kNoCode)
    dictionary_[next_entry_idx_++] = {prev_code_, prev_code_first_byte_};
}

}  // namespace image
