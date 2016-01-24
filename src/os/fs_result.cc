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

#include "os/fs_result.h"

#include <cstring>

namespace os {

// static
FsResult FsResult::Ok() {
  return FsResult();
}

// static
FsResult FsResult::Error(OsError os_error,
                         base::StringPiece op,
                         base::StringPiece path) {
  FsResult result;
  result.os_error_ = os_error;
  result.op_ = op.as_string();
  result.path_ = path.as_string();
  return result;
}

// static
FsResult FsResult::Eof() {
  FsResult result;
  result.eof_ = true;
  return result;
}

// static
FsResult FsResult::Read(size_t n) {
  FsResult result;
  result.n_ = n;
  return result;
}

// static
FsResult FsResult::Write(size_t n) {
  FsResult result;
  result.n_ = n;
  return result;
}

FsResult::FsResult() {}

FsResult::~FsResult() {}

io::IoResult FsResult::ToIoResult() const {
  if (eof())
    return io::IoResult::Eof();

  if (!ok()) {
    if (os_error_.code() == EAGAIN || os_error_.code() == EWOULDBLOCK)
      return io::IoResult::Pending();

    return io::IoResult::Error(ToString());
  }

  // No matter, read or write for now.
  return io::IoResult::Read(n_);
}

std::string FsResult::ToString() const {
  if (ok())
    return std::string();

  return path_ + ": " + op_ + " returned error: " + os_error_.ToString();
}

}  // namespace os
