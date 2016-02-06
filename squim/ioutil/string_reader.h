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

#ifndef SQUIM_IOUTIL_STRING_READER_H_
#define SQUIM_IOUTIL_STRING_READER_H_

#include <string>

#include "squim/io/reader.h"

namespace ioutil {

class StringReader : public io::Reader {
 public:
  explicit StringReader(const std::string& contents);

  io::IoResult Read(io::Chunk* chunk) override;

 private:
  const std::string& contents_;
  size_t offset_ = 0;
};

}  // namespace ioutil

#endif  // SQUIM_IOUTIL_STRING_READER_H_
