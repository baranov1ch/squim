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

#ifndef IMAGE_CODECS_GIF_GIF_IMAGE_PARSER_H_
#define IMAGE_CODECS_GIF_GIF_IMAGE_PARSER_H_

#include <functional>
#include <memory>

#include "image/codecs/gif/gif_image.h"
#include "image/result.h"

namespace io {
class BufferWriter;
class BufReader;
}

namespace image {

class GifImage::Parser {
 public:
  Parser(io::BufReader* source, GifImage* image);
  ~Parser();

  Result ParseHeader();
  Result Parse();

  bool header_complete() const { return header_complete_; }
  bool complete() const { return complete_; }

 private:
  using Handler = std::function<Result()>;

  Result ParseVersion();
  Result ParseLogicalScreenDescriptor();
  Result ParseGlobalColorTable();
  Result ParseBlockType();
  Result ParseExtensionType();
  Result ParseControlExtension();
  Result ParseApplicationExtension();
  Result ParseSubBlockLength(Handler block_handler);
  Result SkipBlock();
  Result ParseImageDescriptor();
  Result ParseLocalColorTable();
  Result ParseMinimumCodeSize();
  Result ReadLZWData();
  Result ParseNetscapeApplicationExtension();
  Result ParseICCPApplicationExtension();
  Result ParseXMPApplicationExtension();

  Result BuildColorTable(ColorTable* color_table);
  Result ConsumeMetadata();
  Result SkipSubBlockHandler(Handler handler);
  Frame::Parser* GetFrameParser();
  Handler BlockHandler(Handler handler);

  void SetupHandlers(Handler start_of_subblock,
                     Handler data_handler,
                     Handler end_of_block);

  Handler handler_;
  Handler end_of_block_handler_;
  Handler start_of_subblock_handler_;
  io::BufReader* source_;
  GifImage* image_;
  bool header_only_ = false;
  bool header_complete_ = false;
  bool complete_ = false;
  Result parser_error_ = Result::Ok();
  size_t remaining_block_length_ = 0;
  std::unique_ptr<Frame::Parser> active_frame_builder_;
  std::unique_ptr<io::BufferWriter> metadata_writer_;
};

}  // namespace image

#endif  // IMAGE_CODECS_GIF_GIF_IMAGE_PARSER_H_
