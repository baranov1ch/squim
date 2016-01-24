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

#ifndef OS_FS_RESULT_H_
#define OS_FS_RESULT_H_

#include <string>

#include "base/strings/string_piece.h"
#include "io/io_result.h"
#include "os/os_error.h"

namespace os {

class FsResult {
 public:
  static FsResult Ok();
  static FsResult Error(OsError os_error,
                        base::StringPiece op,
                        base::StringPiece path);
  static FsResult Eof();
  static FsResult Read(size_t n);
  static FsResult Write(size_t n);

  ~FsResult();

  std::string ToString() const;
  io::IoResult ToIoResult() const;

  const std::string& path() const { return path_; }
  const std::string& op() const { return op_; }

  size_t n() const { return n_; }
  bool ok() const { return os_error_.ok(); }
  bool eof() const { return eof_; }

  OsError os_error() const { return os_error_; }

 private:
  FsResult();

  OsError os_error_ = OsError::Ok();
  std::string op_;
  std::string path_;
  size_t n_ = 0;
  bool eof_ = false;
};

}  // namespace os

#endif  // OS_FS_RESULT_H_
