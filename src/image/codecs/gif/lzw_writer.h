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

#ifndef IMAGE_CODECS_GIF_LZW_WRITER_H_
#define IMAGE_CODECS_GIF_LZW_WRITER_H_

#include <functional>
#include <unordered_map>
#include <vector>

#include "base/logging.h"
#include "io/io_result.h"

namespace io {
class Chunk;
}

namespace std {
template <> struct hash<std::vector<uint8_t>> {
  size_t operator()(const std::vector<uint8_t>& x) const {
    // FNV-1a hash
    const size_t kFNV32Prime = 0x01000193;
    size_t hval = 0x811c9dc5;
    for (uint8_t c : x) {
      hval ^= static_cast<size_t>(c);
      hval *= kFNV32Prime;
    }
    return hval;
  }
};
}

namespace image {

// See http://www.matthewflickinger.com/lab/whatsinagif/lzw_image_data.asp
// for details.
// No comments here, since it is test-only class.
class LZWWriter {
 public:
  LZWWriter();
  ~LZWWriter();

  bool Init(size_t data_size, size_t output_chunk_size, std::function<bool(uint8_t*, size_t)> output_cb);

  io::IoResult Write(io::Chunk* chunk);
  io::IoResult Write(const uint8_t* data, size_t len);
  io::IoResult Finish();

 private:
  static const size_t kMaxCodeSize = 12;
  static constexpr size_t kMaxDictionarySize = 1 << kMaxCodeSize;  // 4096

  class CodeStream {
   public:
    CodeStream();

    void Init(size_t data_size, size_t output_chunk_size, std::function<bool(uint8_t*, size_t)> output_cb);

    bool Write(uint16_t code);
    bool Finish();

    bool CodeFitsCurrentCodesize(size_t code) const;
    void IncreaseCodesize();
    void ResetCodesize(size_t data_size);

   private:
    uint32_t buffer_ = 0;
    size_t bits_in_buffer_ = 0;
    size_t codesize_ = 0;

    size_t output_chunk_size_ = 0;
    std::function<bool(uint8_t*, size_t)> output_cb_;
    std::vector<uint8_t> output_;
    std::vector<uint8_t>::iterator output_it_;
  };

  void Clear();

  using IndexBuffer = std::vector<uint8_t>;
  using CodeTable = std::unordered_map<IndexBuffer, uint16_t>;
  IndexBuffer index_buffer_;
  CodeTable code_table_;
  CodeStream code_stream_;

  uint16_t last_code_in_index_buffer_ = 0;
  uint16_t next_code_ = 0;
  uint16_t clear_code_ = 0;
  uint16_t eoi_ = 0;

  size_t data_size_ = 0;
  bool started_ = false;
};

}  // namespace image

#endif  // IMAGE_CODECS_GIF_LZW_WRITER_H_
