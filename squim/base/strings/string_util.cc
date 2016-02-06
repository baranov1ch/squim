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

#include <cstdint>

#include "squim/base/strings/string_piece.h"

namespace base {

base::StringPiece StringFromBytes(const uint8_t* bytes, uint64_t len) {
  return base::StringPiece(reinterpret_cast<const char*>(bytes), len);
}

uint8_t* BytesFromConstChar(const char* data) {
  return reinterpret_cast<uint8_t*>(const_cast<char*>(data));
}

}  // namespace base
