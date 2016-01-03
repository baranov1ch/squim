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

#ifndef IMAGE_CODECS_GIF_GIF_IMAGE_H_
#define IMAGE_CODECS_GIF_GIF_IMAGE_H_

#include <array>
#include <functional>
#include <memory>
#include <vector>

#include "image/image_metadata.h"
#include "image/pixel.h"
#include "image/result.h"

namespace io {
class BufferWriter;
class BufReader;
}

namespace image {

class LZWReader;

class GifImage {
 public:
  enum class DisposalMethod {
    kNotSpecified,
    kKeep,
    kOverwriteBgcolor,
    kOverwritePrevious,
  };

  static const size_t kInifiniteLoop = 0xFFFFFFFF;

  class ColorTable {
   public:
    static const int kNumBytesPerEntry = 3;

    ColorTable(size_t table_size);
    ~ColorTable();

    size_t expected_size() const { return expected_size_; }
    size_t size() const { return table_.size(); }

    void AddColor(uint8_t r, uint8_t g, uint8_t b);
    const RGBPixel GetColor(size_t idx) const;

   private:
    std::vector<std::array<uint8_t, kNumBytesPerEntry>> table_;
    size_t expected_size_;
  };

  class Frame {
   public:
    class Builder {
     public:
      Builder();
      ~Builder();

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

      std::unique_ptr<Frame> Build();

     private:
      std::unique_ptr<Frame> frame_;
      std::unique_ptr<LZWReader> lzw_reader_;
      uint16_t current_row_;
      size_t interlace_pass_;
    };

    Frame();
    ~Frame();

    const ColorTable* GetColorTable() const;
    uint8_t GetPixel(uint16_t x, uint16_t y) const;

    uint16_t width() const { return width_; }
    uint16_t height() const { return height_; }
    uint16_t x_offset() const { return x_offset_; }
    uint16_t y_offset() const { return y_offset_; }
    size_t transparent_pixel() const { return transparent_pixel_; }
    size_t duration() const { return duration_; }
    DisposalMethod disposal_method() const { return disposal_method_; }
    bool is_progressive() const { return is_progressive_; }

   private:
    void SetRow(uint16_t nrow, uint8_t* row_data);

    uint16_t width_;
    uint16_t height_;
    uint16_t x_offset_;
    uint16_t y_offset_;
    size_t transparent_pixel_;
    size_t duration_;
    DisposalMethod disposal_method_;
    bool is_progressive_;

    std::unique_ptr<uint8_t[]> data_;

    std::unique_ptr<ColorTable> local_color_table_;
    const ColorTable* global_color_table_;
  };

  class Parser {
   public:
    Parser(io::BufReader* source, GifImage* image);
    ~Parser();

    Result ParseHeader();
    Result Parse();

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
    Frame::Builder* GetFrameBuilder();
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
    size_t remaining_block_length_ = 0;
    std::unique_ptr<Frame::Builder> active_frame_builder_;
    std::unique_ptr<io::BufferWriter> metadata_writer_;
  };

  const ColorTable* global_color_table() const {
    return global_color_table_.get();
  }

  int version() const { return version_; }
  uint16_t screen_width() const { return screen_width_; }
  uint16_t screen_height() const { return screen_height_; }
  uint8_t color_resolution() const { return color_resolution_; }

  size_t background_color_index() const { return background_color_index_; }
  size_t loop_count() const { return loop_count_; }

 private:
  std::vector<std::unique_ptr<Frame>> frames_;
  std::unique_ptr<ColorTable> global_color_table_;
  uint16_t screen_width_;
  uint16_t screen_height_;
  size_t loop_count_;
  int version_;
  uint8_t color_resolution_;
  size_t background_color_index_;
  ImageMetadata metadata_;
};

}  // namespace image

#endif  // IMAGE_CODECS_GIF_GIF_IMAGE_H_
