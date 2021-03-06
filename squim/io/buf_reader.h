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

#ifndef SQUIM_IO_BUF_READER_H_
#define SQUIM_IO_BUF_READER_H_

#include <cstdint>
#include <memory>

#include "squim/io/io_result.h"

namespace io {

class BufferedSource;

// Reads data from the underlying |source_|. Tries not to do any copies.
class BufReader {
 public:
  static std::unique_ptr<BufReader> CreateEmpty();

  BufReader(std::unique_ptr<BufferedSource> source);
  ~BufReader();

  // Sets |out| to the largest continuous piece of data available in |source_|.
  // Advances the offset for returned IoResult::n() bytes.
  // Do not copies any data, just returns a pointer.
  IoResult ReadSome(uint8_t** out);

  // Sets |out| to continuous piece of data of the size |n| or less.
  // Advances the offset for returned IoResult::n() bytes.
  // Do not copies any data, just returns a pointer.
  IoResult ReadAtMostN(uint8_t** out, size_t n);

  // Sets |out| to continuous piece of data of the size |n|.
  // Advances the offset for returned IoResult::n() bytes.
  // NOTE: If |source_| does not have enough continuous data, some of the chunks
  // will be merged, so data copies may occur during this call.
  // Returns error if |source_| ended (got EOF) earlier.
  IoResult ReadN(uint8_t** out, size_t n);

  // Copies into |out| piece of data of the size |n|.
  // Advances the offset for returned IoResult::n() bytes (which is |n|).
  // Returns error if |source_| ended (got EOF) earlier.
  IoResult ReadNInto(uint8_t* out, size_t n);

  template <size_t N>
  IoResult ReadNInto(uint8_t(&out)[N]) {
    return ReadNInto(out, N);
  }

  // Copies into |out| piece of data of the size |n|.
  // Does not advance the offset.
  IoResult PeekNInto(uint8_t* out, size_t n);

  template <size_t N>
  IoResult PeekNInto(uint8_t(&out)[N]) {
    return PeekNInto(out, N);
  }

  // Skips |n| bytes from underlying source.
  IoResult SkipN(size_t n);

  // Unreads n bytes from the buffer. Returns the number of bytes unread.
  size_t UnreadN(size_t n);

  BufferedSource* source() { return source_.get(); }
  size_t offset() const;
  bool HaveSome() const;

 private:
  std::unique_ptr<BufferedSource> source_;
};

}  // namespace io

#endif  // SQUIM_IO_BUF_READER_H_
