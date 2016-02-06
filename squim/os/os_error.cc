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

#include "squim/os/os_error.h"

#include <cstring>

namespace os {

// static
OsError OsError::Ok() {
  return OsError(0);
}

// static
OsError OsError::Error(int error_code) {
  return OsError(error_code);
}

OsError::OsError(int os_error_code) : os_error_code_(os_error_code) {}

OsError::~OsError() {}

const char* OsError::ToString() const {
  return strerror(os_error_code_);
}

}  // namespace os
