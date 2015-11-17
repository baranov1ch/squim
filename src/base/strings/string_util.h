#ifndef BASE_STRINGS_STRING_UTIL_H_
#define BASE_STRINGS_STRING_UTIL_H_

#include <cstdint>

#include "base/strings/string_piece.h"

namespace base {

base::StringPiece StringFromBytes(const uint8_t* bytes, uint64_t len) {
  return base::StringPiece(reinterpret_cast<const char*>(bytes), len);
}

}  // namespace base

#endif  // BASE_STRINGS_STRING_UTIL_H_
