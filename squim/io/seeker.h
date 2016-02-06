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

#ifndef SQUIM_IO_SEEKER_H_
#define SQUIM_IO_SEEKER_H_

#include "squim/io/io_result.h"

namespace io {

class Seeker {
 public:
  enum class Whence {
    kSet = 0,
    kCur = 1,
    kEnd = 2,
  };

  virtual IoResult Seek(size_t offset, Whence whence) = 0;

  virtual ~Seeker() {}
};

}  // namespace io

#endif  // SQUIM_IO_SEEKER_H_
