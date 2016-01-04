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

#ifndef IMAGE_CODECS_GIF_GIF_IMAGE_FRAME_PARSER_H_
#define IMAGE_CODECS_GIF_GIF_IMAGE_FRAME_PARSER_H_

#include <memory>

#include "image/codecs/gif/gif_image.h"
#include "image/result.h"

namespace image {

class LZWReader;

class GifImage::Frame::Parser {
 public:
  Parser();
  ~Parser();

  void SetGlobalColorTable(const ColorTable* global_color_table);
  void SetFrameGeometry(uint16_t x_offset,
                        uint16_t y_offset,
                        uint16_t width,
                        uint16_t height);
  void SetProgressive(bool progressive);
  void SetTransparentPixel(size_t value);
  void SetDuration(size_t duration);
  void SetDisposalMethod(DisposalMethod method);
  bool InitDecoder(uint8_t minimum_code_size);
  Result ProcessImageData(uint8_t* data, size_t size);

  void CreateLocalColorTable(size_t size);
  ColorTable* GetLocalColorTable();

  std::unique_ptr<Frame> ReleaseFrame();

 private:
  bool OutputRow(uint8_t* data, size_t size);

  std::unique_ptr<Frame> frame_;
  std::unique_ptr<LZWReader> lzw_reader_;
  uint16_t current_row_ = 0;
  size_t interlace_pass_ = 0;
};

}  // namespace image

#endif  // IMAGE_CODECS_GIF_GIF_IMAGE_FRAME_PARSER_H_
