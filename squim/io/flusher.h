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

#ifndef SQUIM_IO_FLUSHER_H_
#define SQUIM_IO_FLUSHER_H_

#include "squim/io/io_result.h"

namespace io {

class Flusher {
 public:
  virtual IoResult Flush() = 0;

  virtual ~Flusher() {}
};

}  // namespace io

#endif  // SQUIM_IO_FLUSHER_H_
