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

#ifndef SQUIM_OS_OS_ERROR_H_
#define SQUIM_OS_OS_ERROR_H_

namespace os {

class OsError {
 public:
  static OsError Ok();
  static OsError Error(int error_code);

  ~OsError();

  const char* ToString() const;
  bool ok() const { return os_error_code_ == 0; }
  int code() const { return os_error_code_; }

 private:
  OsError(int os_error_code);

  int os_error_code_ = 0;
};

}  // namespace os

#endif  // SQUIM_OS_OS_ERROR_H_
