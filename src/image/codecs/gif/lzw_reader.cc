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

#include "image/codecs/gif/lzw_reader.h"

#include <algorithm>
#include <cstring>

#include "base/logging.h"
#include "io/chunk.h"

namespace image {

LZWReader::CodeStream::CodeStream() {}

void LZWReader::CodeStream::SetupBuffer(uint8_t* buffer, size_t size) {
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
  return !(code & codemask_);
}

void LZWReader::CodeStream::IncreaseCodesize() {
  ResetCodesize(codesize_ + 1);
}

void LZWReader::CodeStream::ResetCodesize(size_t data_size) {
  codesize_ = data_size + 1;
  codemask_ = (1 << codesize_) - 1;
}

LZWReader::LZWReader() {}

LZWReader::~LZWReader() {}

bool LZWReader::Init(size_t data_size, size_t output_chunk_size, std::function<bool(uint8_t*, size_t)> output_cb) {
  if (data_size > kMaxCodeSize)
    return false;

  data_size_ = data_size;
  dictionary_.reserve(kMaxDictionarySize);

  clear_code_ = 1 << data_size_;
  eoi_ = clear_code_ + 1;

  Clear();

  // Fill trivial codes.
  for (size_t i = 0; i < clear_code_; ++i)
    dictionary_[i] = { kNoCode, 1, static_cast<uint8_t>(i) };

  const size_t kMaxBytes = kMaxDictionarySize - 1;
  output_chunk_size_ = output_chunk_size;
  output_.reserve(output_chunk_size_ - 1 + kMaxBytes);
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

io::IoResult LZWReader::Decode(uint8_t* data, size_t size) {
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
      return io::IoResult::Eof();
    }

    if (!OutputCodeToStream(code))
      return io::IoResult::Error();

    UpdateDictionary();

    if (!code_stream_.CodeFitsCurrentCodesize(next_entry_idx_) &&
        next_entry_idx_ < kMaxDictionarySize) {
      code_stream_.IncreaseCodesize();
    }

    prev_code_ = code;

    // Output to the client as much as we can by chunks of |output_chunk_size_|.
    auto chunk_start = output_.begin();
    for (; chunk_start + output_chunk_size_ <= output_it_; chunk_start += output_chunk_size_) {
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
  return io::IoResult::Read(size);
}

bool LZWReader::OutputCodeToStream(uint16_t code) {
  // Output works from right-to-left. We start from the last byte in sequence
  // coded by |code|, and follow prefix references from the dictionary to
  // reconstruct entire byte sequence (remember that every byte string stored in
  // LZW dictionary is some another string of that dictionary + one byte and
  // dictionary is stored as a collection of prefix references + |value| byte).
  // Example:
  // Here is our dictionary, |data_size_| = 2, thus we have 4 static entries:
  // [
  //   {X, 1, '0'}, - X means no prefix code, 1 - length, '0' - value
  //   {X, 1, '1'}, - same for the rest 3 static codes, incrementing value
  //   {X, 1, '2'}, - yada-yada
  //   {X, 1, '3'}, - ah, at last.
  //   {X, X, X}, - placeholder, clear-code
  //   {X, X, X}, - placeholder, end-of-information code.
  // ]
  // |next_entry_idx_| = 6, |prev_code_| empty, |prev_code_first_byte_| empty.
  // We get first code, which is '1' for example and set |prev_code_| Nothing
  // is added to the dictionary, since |prev_code_| was empty. But now it's
  // not:)
  // |next_entry_idx_| = 6, |prev_code_| = 1, which points to {X, 1, '1'},
  // |prev_code_first_byte_| = '1'
  //
  // Next code in stream is '6'. It is not in the dictionary, thus we should
  // output |prev_code_| value plus its first byte. Luckily, |prev_code_|
  // encodes single byte ('1'), an we can just put '11' to output.
  // UpdateDictionary() will append {1, 2, '1'} entry to the end of the
  // |dictionary_|:
  // [
  //   {X, 1, '0'}, - X means no prefix code, 1 - length, 0 - value
  //   {X, 1, '1'}, - same for the rest 3 static codes, incrementing value
  //   {X, 1, '2'}, - yada-yada
  //   {X, 1, '3'}, - ah, at last.
  //   {X, X, X}, - placeholder, clear-code
  //   {X, X, X}, - placeholder, end-of-information code.
  //   {1, 2, '1'}, - code of 2 symbols ('1', '1')
  // ]
  // |next_entry_idx_| = 7, |prev_code_| = 6, meaning it points to {1, 2, '1'}.
  // |prev_code_first_byte_| = '1' 
  //
  // Incoming 7! Again, it is not in our dictionary. |prev_code_| is 6, so we
  // should output '111'. How we do it? First, move |output_it_| to the end of
  // assumed output sequence, i.e. for 3 positions
  //   |   output_it_               output_it_ |
  //   v                    ===>               v
  // [..][..][..][..][..]         [..][..][..][..][..]
  // Output |prev_code_first_byte_| to the last position:
  //           | output_it_ 
  //           v
  // [..][..]['1'][..][..]
  // Next, we have to putput |prev_code_| which is 6, which is ('11')
  // we look at {1, 2, '1'}, and see that its |suffix| is '1', so output it:
  //       | output_it_ 
  //       v
  // [..]['1']['1'][..][..]
  // Then, we follow |prefix_code| of the 6-th entry, which is 1, so we end up
  // in trivial code {X, 1, '1'}, output it and stop:
  //   | output_it_ 
  //   v
  // ['1']['1']['1'][..][..]
  // Finally, we move |output_it_| to the next free position:
  //                  | output_it_ 
  //                  v
  // ['1']['1']['1'][..][..]
  // At last, we add new entry {6, 3, '1'} to the |dictionary_|, and update state:
  // |next_entry_idx_| = 8, |prev_code_| = 7, meaning it points to {6, 3, '1'}.
  // |prev_code_first_byte_| = '1'
  // Now, we're ready to proceed with the next code.
  size_t code_length = 0;
  if (code < next_entry_idx_) {
    // The code is already in the dictionary - just output it.
    code_length = dictionary_[code].length;

    // Move iterator to the end of the assumed output.
    output_it_ += code_length;
  } else if (code == next_entry_idx_ && prev_code_ != kNoCode) {
    // New code - output previous code value + first its byte.
    code_length = dictionary_[prev_code_].length + 1;

    // Move iterator to the end of the assumed output.
    output_it_ += code_length;

    // Write first byte of the previous symbol's coded sequence to the end of
    // the output and move reference to the previous code.
    *--output_it_ = prev_code_first_byte_;
    code = prev_code_;
  } else {
    // This is an invalid code. The dictionary is just initialized
    // and the code is incomplete. We don't know how to handle
    // this case.
    return false;
  }

  // Follow backreferences and reconstruct the output until we hit
  // trivial code (i.e. |code| is less than |clear_code_|).
  while (code >= clear_code_) {
    auto& entry = dictionary_[code];
    *--output_it_ = entry.suffix;
    code = entry.prefix_code;
  }

  // Output the first byte of the sequence (which MUST correspond to trivial
  // code).
  DCHECK_LT(code, clear_code_);
  auto& entry = dictionary_[code];
  *--output_it_ = entry.suffix;
  prev_code_first_byte_ = entry.suffix;

  // Return iterator back to the end of written byte sequence.
  output_it_ += code_length;

  return true;
}

void LZWReader::UpdateDictionary() {
  // Add |prev_code_| + |prev_code_first_byte_| to the code table.
  if (next_entry_idx_ < kMaxDictionarySize && prev_code_ != kNoCode) {
    uint16_t new_len = dictionary_[prev_code_].length + 1;
    dictionary_[next_entry_idx_++] = { prev_code_, new_len, prev_code_first_byte_ };
  }
}

}  // namespace image
