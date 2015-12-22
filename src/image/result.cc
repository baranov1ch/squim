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

#include "image/result.h"

namespace image {

// static
const char* Result::CodeToString(Code code) {
  // TODO: macro generator.
  switch (code) {
    case Code::kOk:
      return "Ok";
    case Code::kImageTooLarge:
      return "ImageTooLarge";
    case Code::kImageTooSmall:
      return "ImageTooSmall";
    case Code::kPending:
      return "Pending";
    case Code::kDecodeError:
      return "DecodeError";
    case Code::kEncodeError:
      return "EncodeError";
    case Code::kUnsupportedFormat:
      return "UnsupportedFormat";
    case Code::kUnexpectedEof:
      return "UnexpectedEof";
    case Code::kIoErrorOther:
      return "IoErrorOther";
    case Code::kDunnoHowToEncode:
      return "DunnoHowToEncode";
    case Code::kReadFrameError:
      return "ReadFrameError";
    case Code::kWriteFrameError:
      return "WriteFrameError";
    case Code::kFailed:
      return "Failed";
    default:
      return "<unknown>";
  }
}

// static
Result Result::Ok() {
  return Result(Code::kOk, std::string(), false);
}

// static
Result Result::Pending() {
  return Result(Code::kPending, std::string(), false);
}

// static
Result Result::Error(Code code) {
  return Result(code, std::string(), false);
}

// static
Result Result::Error(Code code, std::string custom_message) {
  return Result(code, custom_message, false);
}

// static
Result Result::Finish(Code code) {
  return Result(code, std::string(), true);
}

// static
Result Result::FromIoResult(io::IoResult io_result, bool eof_ok) {
  if (io_result.ok())
    return Result::Ok();

  if (io_result.pending())
    return Result::Pending();

  if (io_result.eof()) {
    if (eof_ok) {
      return Result::Finish(Code::kOk);
    }
    return Result::Error(Code::kUnexpectedEof);
  }

  return Result::Error(Code::kIoErrorOther);
}

Result::Result(Code code, std::string custom_message, bool finished)
    : code_(code), custom_message_(custom_message), finished_(finished) {}

std::ostream& operator<<(std::ostream& os, Result::Code code) {
  os << Result::CodeToString(code);
  return os;
}

}  // namespace image
