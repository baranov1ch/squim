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

#ifndef OS_OS_ERROR_H_
#define OS_OS_ERROR_H_

namespace os {

class OsError {
 public:
  static OsError Ok();
  static OsError Error(int error_code);

  const char* ToString() const;

 private:
  int os_error_code_;
  std::string message_;
};

}  // namespace os

#endif  // OS_OS_ERROR_H_
