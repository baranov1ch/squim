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

#ifndef IOUTIL_READ_UTIL_H_
#define IOUTIL_READ_UTIL_H_

#include "io/chunk.h"
#include "io/io_result.h"

namespace io {
class Reader;
class Writer;
}

namespace ioutil {

io::IoResult Copy(io::Writer* dst, io::Reader* src);

io::IoResult ReadFull(io::Reader* reader, io::ChunkList* out);

io::IoResult ReadFull(io::Reader* reader, io::ChunkPtr* out);

}  // namespace ioutil

#endif  // IOUTIL_READ_UTIL_H_
