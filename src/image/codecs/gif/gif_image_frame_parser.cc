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

#include "image/codecs/gif/gif_image_frame_parser.h"

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/codecs/gif/lzw_reader.h"

namespace image {

GifImage::Frame::Parser::Parser() {
  frame_ = base::make_unique<Frame>();
}

GifImage::Frame::Parser::~Parser() {}

void GifImage::Frame::Parser::SetGlobalColorTable(
    const ColorTable* global_color_table) {
  frame_->global_color_table_ = global_color_table;
}

void GifImage::Frame::Parser::SetFrameGeometry(uint16_t x_offset,
                                               uint16_t y_offset,
                                               uint16_t width,
                                               uint16_t height) {
  frame_->x_offset_ = x_offset;
  frame_->y_offset_ = y_offset;
  frame_->width_ = width;
  frame_->height_ = height;
}

void GifImage::Frame::Parser::SetProgressive(bool progressive) {
  frame_->is_progressive_ = progressive;
}

void GifImage::Frame::Parser::SetTransparentPixel(size_t value) {
  frame_->transparent_pixel_ = value;
}

void GifImage::Frame::Parser::SetDuration(size_t duration) {
  frame_->duration_ = duration;
}

void GifImage::Frame::Parser::SetDisposalMethod(DisposalMethod method) {
  frame_->disposal_method_ = method;
}

bool GifImage::Frame::Parser::InitDecoder(uint8_t minimum_code_size) {
  lzw_reader_ = base::make_unique<LZWReader>();
  auto output = [this](uint8_t* data, size_t size) -> bool {
    return OutputRow(data, size);
  };
  return lzw_reader_->Init(minimum_code_size, frame_->width_, output);
}

Result GifImage::Frame::Parser::ProcessImageData(uint8_t* data, size_t size) {
  auto result = lzw_reader_->Decode(data, size);
  if (result.ok()) {
    DCHECK_EQ(size, result.n());
  }

  if (result.eof()) {
    if (current_row_ != frame_->height() - 1) {
      return Result::Error(Result::Code::kDecodeError, "Image data too short");
    } else {
      return Result::Ok();
    }
  }
  return Result::FromIoResult(result, false);
}

bool GifImage::Frame::Parser::OutputRow(uint8_t* data, size_t size) {
  if (size < frame_->width())
    return false;

  if (current_row_ >= frame_->height())
    return false;

  if (!frame_->data_) {
    frame_->data_.reset(new uint8_t[frame_->width() * frame_->height()]);
  }

  frame_->SetRow(current_row_, data);
  if (!frame_->is_progressive()) {
    current_row_++;
  } else {
    static const struct InterlacePass {
      size_t start;
      size_t step;
    } kInterlace[] = {{0, 8}, {4, 8}, {2, 4}, {1, 2}};
    current_row_ += kInterlace[interlace_pass_].step;
    if (current_row_ >= frame_->height() && interlace_pass_ < 3) {
      interlace_pass_++;
      current_row_ = kInterlace[interlace_pass_].start;
    }
  }

  return true;
}

void GifImage::Frame::Parser::CreateLocalColorTable(size_t size) {
  frame_->local_color_table_.reset(new ColorTable(size));
}

GifImage::ColorTable* GifImage::Frame::Parser::GetLocalColorTable() {
  return frame_->local_color_table_.get();
}

std::unique_ptr<GifImage::Frame> GifImage::Frame::Parser::ReleaseFrame() {
  return std::move(frame_);
}

}  // namespace image
