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

#include "io/io_result.h"

namespace io {

// static
IoResult IoResult::Pending() {
  return IoResult(IoResultCode::kPending);
}

// static
IoResult IoResult::Read(size_t nread) {
  return IoResult(nread);
}

// static
IoResult IoResult::Eof() {
  return IoResult(IoResultCode::kEof);
}

// static
IoResult IoResult::Error() {
  return IoResult(IoResultCode::kError);
}

IoResult::IoResult(IoResultCode code) : code_(code), n_(0u) {}

IoResult::IoResult(size_t n) : code_(IoResultCode::kOk), n_(n) {}

}  // namespace io
