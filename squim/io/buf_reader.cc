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

#include "squim/io/buf_reader.h"

#include <cstring>

#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/io/buffered_source.h"

namespace io {

// static
std::unique_ptr<BufReader> BufReader::CreateEmpty() {
  return base::make_unique<BufReader>(base::make_unique<BufferedSource>());
}

BufReader::BufReader(std::unique_ptr<BufferedSource> source)
    : source_(std::move(source)) {}

BufReader::~BufReader() {}

IoResult BufReader::ReadSome(uint8_t** out) {
  if (source_->EofReached())
    return IoResult::Eof();

  if (!source_->HaveSome())
    return IoResult::Pending();

  auto nread = source_->ReadSome(out);
  return IoResult::Read(nread);
}

IoResult BufReader::ReadAtMostN(uint8_t** out, size_t n) {
  if (source_->EofReached())
    return IoResult::Eof();

  if (!source_->HaveSome())
    return IoResult::Pending();

  auto nread = source_->ReadAtMostN(out, n);
  return IoResult::Read(nread);
}

IoResult BufReader::ReadN(uint8_t** out, size_t n) {
  if (source_->EofReached())
    return IoResult::Eof();

  if (!source_->HaveN(n))
    return IoResult::Pending();

  auto nread = source_->ReadN(out, n);
  return IoResult::Read(nread);
}

IoResult BufReader::ReadNInto(uint8_t* out, size_t n) {
  CHECK(out);

  if (source_->EofReached())
    return IoResult::Eof();

  if (!source_->HaveN(n))
    return IoResult::Pending();

  size_t offset = 0;
  size_t left = n;
  while (left > 0) {
    uint8_t* tmp;
    auto nread = source_->ReadAtMostN(&tmp, left);
    std::memcpy(out + offset, tmp, nread);
    left -= nread;
    offset += nread;
  }
  return IoResult::Read(n);
}

IoResult BufReader::PeekNInto(uint8_t* out, size_t n) {
  auto result = ReadNInto(out, n);
  if (result.ok()) {
    CHECK_EQ(n, result.n());
    UnreadN(n);
  }
  return result;
}

size_t BufReader::UnreadN(size_t n) {
  return source_->UnreadN(n);
}

IoResult BufReader::SkipN(size_t n) {
  if (source_->EofReached())
    return IoResult::Eof();

  if (!source_->HaveN(n))
    return IoResult::Pending();

  size_t left = n;
  while (left > 0) {
    uint8_t* tmp;
    auto nread = source_->ReadAtMostN(&tmp, left);
    left -= nread;
  }
  return IoResult::Read(n);
}

size_t BufReader::offset() const {
  return source_->offset();
}

bool BufReader::HaveSome() const {
  return source_->HaveSome();
}

}  // namespace io
