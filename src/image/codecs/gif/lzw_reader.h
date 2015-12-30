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

#ifndef IMAGE_CODECS_GIF_LZW_READER_H_
#define IMAGE_CODECS_GIF_LZW_READER_H_

#include <functional>
#include <vector>

#include "io/io_result.h"

namespace io {
class Chunk;
}

namespace image {

// See http://www.matthewflickinger.com/lab/whatsinagif/lzw_image_data.asp
// for details.
class LZWReader {
 public:
  LZWReader();
  ~LZWReader();

  bool Init(size_t data_size, size_t output_chunk_size, std::function<bool(uint8_t*, size_t)> output_cb);

  io::IoResult Decode(io::Chunk* chunk);
  io::IoResult Decode(uint8_t* data, size_t size);

 private:
  static const size_t kMaxCodeSize = 12;
  static constexpr size_t kMaxDictionarySize = 1 << kMaxCodeSize;  // 4096
  static const uint16_t kNoCode = 0xFFFF;

  void Clear();
  bool OutputCodeToStream(uint16_t code);
  void UpdateDictionary();

  // Reads varable-length codes from byte stream.
  class CodeStream {
   public:
    CodeStream();

    void SetupBuffer(uint8_t* buffer, size_t size);

    bool ReadNext(uint16_t* code);

    bool CodeFitsCurrentCodesize(size_t code) const;
    void IncreaseCodesize();
    void ResetCodesize(size_t data_size);

   private:
    uint8_t* input_ = nullptr;
    size_t input_size_ = 0;

    uint32_t buffer_ = 0;
    size_t unread_bits_ = 0;

    size_t codesize_ = 0;
    uint32_t codemask_ = 0;
  };

  // Each entry in LZW dictionary is some previous entry + one byte. Thus,
  // entry can be stored as (reference_to_prev, new_byte) pair. There can
  // up to 4096 entries, so |prefix_code|:uint16_t is enough to address them.
  // |suffix|:uint8_t is the new byte, |length|:uint16_t is the length of the
  // entry, just an auxiliary variable to ease code writing into output, since
  // code is reconstructed from the end (see OutputCodeToStream()).
  // The dictionary as an array contains static trivial part (which contains
  // 2 ^ |data_size_| trivial symbols, 2 empty entries (for clear-code and
  // end-of-information code), and dynamically constructed entries. Value of the
  // code is the index in array, so it is O(1)-addressable. It takes O(length)
  // to reconstruct the byte sequence coded by the |code|, following
  // |prefix_code| references.
  struct Entry {
    uint16_t prefix_code;
    uint16_t length;
    uint8_t suffix;
  };
  std::vector<Entry> dictionary_;
  size_t next_entry_idx_;

  // Previous code and its first byte. Used for dictionary construction.
  // |prev_code_first_byte_| is here for optimization only, we can as well
  // reconstruct it from |prev_code_| walking through prefixes in |dictionary_|.
  uint16_t prev_code_;
  uint8_t prev_code_first_byte_;

  // Gif LZW special codes.
  uint16_t clear_code_;
  uint16_t eoi_;

  CodeStream code_stream_;

  size_t data_size_;

  size_t output_chunk_size_;

  std::function<bool(uint8_t*, size_t)> output_cb_;
  std::vector<uint8_t> output_;
  std::vector<uint8_t>::iterator output_it_;
};

}  // namespace image

#endif  // IMAGE_CODECS_GIF_LZW_READER_H_
