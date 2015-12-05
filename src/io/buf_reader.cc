#include "io/buf_reader.h"

#include <cstring>

#include "glog/logging.h"
#include "io/buffered_source.h"

namespace io {

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

}  // namespace io
