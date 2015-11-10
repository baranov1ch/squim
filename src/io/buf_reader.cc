#include "io/buf_reader.h"
#include "io/buffered_source.h"

namespace io {

BufReader::BufReader(std::unique_ptr<BufferedSource> source) : source_(std::move(source)) {}

BufReader::~BufReader() {}

}  // namespace io
