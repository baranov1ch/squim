/*
 * Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#include "ioutil/string_reader.h"

#include <cstring>

#include "io/chunk.h"

namespace ioutil {

StringReader::StringReader(const std::string& contents) : contents_(contents) {}

io::IoResult StringReader::Read(io::Chunk* chunk) {
  if (offset_ >= contents_.size())
    return io::IoResult::Eof();

  auto eff_len = std::min(chunk->size(), contents_.size() - offset_);
  std::memcpy(chunk->data(), contents_.data() + offset_, eff_len);
  offset_ += eff_len;
  return io::IoResult::Read(eff_len);
}

}  // namespace ioutil
