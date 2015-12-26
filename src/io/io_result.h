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

#ifndef IO_IO_RESULT_H_
#define IO_IO_RESULT_H_

#include <cstddef>
#include <cstdint>

namespace io {

enum class IoResultCode {
  kOk,
  kPending,
  kEof,
  kError,
  // TODO: more errors.
};

class IoResult {
 public:
  static IoResult Pending();
  static IoResult Read(size_t n);
  static IoResult Write(size_t n);
  static IoResult Eof();
  static IoResult Error();

  bool ok() const { return code_ == IoResultCode::kOk; }
  bool pending() const { return code_ == IoResultCode::kPending; }
  size_t n() const { return n_; }
  bool eof() const { return code_ == IoResultCode::kEof; }
  bool error() const { return code_ == IoResultCode::kError; }
  IoResultCode code() const { return code_; }

 private:
  IoResult(IoResultCode code);
  IoResult(size_t nread);

  IoResultCode code_;
  size_t n_;
};

}  // namespace io

#endif  // IO_IO_RESULT_H_
