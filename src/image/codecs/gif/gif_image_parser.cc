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

#include "image/codecs/gif/gif_image_parser.h"

#include <cstring>
#include <limits>

#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "image/codecs/gif/gif_image_frame_parser.h"
#include "io/buf_reader.h"
#include "io/buffer_writer.h"
#include "io/buffered_source.h"

namespace image {

#define HANDLER(func) std::bind(&GifImage::Parser::func, this)

namespace {

class Reader {
 public:
  Reader(uint8_t* data) : data_(data) {}

  // Get a 16-bit value stored in little-endian format.
  uint16_t ReadUint16() {
    uint16_t result = (data_[1] << 8) | data_[0];
    data_ += 2;
    return result;
  }
  uint8_t ReadByte() { return *data_++; }

 private:
  uint8_t* data_;
};

Result FinishParse() {
  return Result::Finish(Result::Code::kOk);
}

}  // namespace

GifImage::Parser::Parser(io::BufReader* source, GifImage* image)
    : source_(source), image_(image) {
  handler_ = HANDLER(ParseVersion);
}

GifImage::Parser::~Parser() {}

GifImage::Frame::Parser* GifImage::Parser::GetFrameParser() {
  if (!active_frame_builder_)
    active_frame_builder_.reset(new Frame::Parser());

  return active_frame_builder_.get();
}

GifImage::Parser::Handler GifImage::Parser::BlockHandler(Handler handler) {
  return std::bind(&GifImage::Parser::ParseSubBlockLength, this, handler);
}

void GifImage::Parser::SetupHandlers(Handler start_of_subblock,
                                     Handler data_handler,
                                     Handler end_of_block) {
  start_of_subblock_handler_ = start_of_subblock;
  handler_ = BlockHandler(data_handler);
  end_of_block_handler_ = end_of_block;
}

Result GifImage::Parser::BuildColorTable(ColorTable* color_table) {
  DCHECK(color_table);
  while (color_table->size() < color_table->expected_size()) {
    uint8_t rgb[ColorTable::kNumBytesPerEntry];
    auto result = source_->ReadNInto(rgb);
    if (!result.ok())
      return Result::FromIoResult(result, false);
    color_table->AddColor(rgb[0], rgb[1], rgb[2]);
  }
  DCHECK_EQ(color_table->expected_size(), color_table->size());
  return Result::Ok();
}

Result GifImage::Parser::ParseHeader() {
  header_only_ = true;
  return Parse();
}

Result GifImage::Parser::Parse() {
  if (!parser_error_.ok())
    return parser_error_;

  if ((header_only_ && header_complete_) || complete_)
    return Result::Finish(Result::Code::kOk);
  auto result = Result::Ok();
  while (result.ok() && !result.finished()) {
    result = handler_();
    if (result.error())
      parser_error_ = result;
  }
  return result;
}

Result GifImage::Parser::ParseVersion() {
  const size_t kLength = 6;
  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);

  if (std::memcmp(data, "GIF89a", kLength)) {
    image_->version_ = 89;
  } else if (std::memcmp(data, "GIF87a", kLength)) {
    image_->version_ = 87;
  } else {
    return Result::Error(Result::Code::kDecodeError, "Unknown GIF version");
  }

  handler_ = HANDLER(ParseLogicalScreenDescriptor);
  return Result::Ok();
}

Result GifImage::Parser::ParseLogicalScreenDescriptor() {
  const size_t kLength = 7;
  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);

  Reader reader(data);
  image_->screen_width_ = reader.ReadUint16();
  image_->screen_height_ = reader.ReadUint16();
  auto packed = reader.ReadByte();
  auto global_table_size = 1 << ((packed & 0x07) + 1);
  if (packed & 0x80 && global_table_size > 0) {
    image_->global_color_table_.reset(new ColorTable(global_table_size));
    handler_ = HANDLER(ParseGlobalColorTable);
  } else {
    handler_ = HANDLER(ParseBlockType);
  }
  auto background_color = reader.ReadByte();
  if (background_color) {
    if (!image_->global_color_table_) {
      LOG(WARNING) << "Background color without global color table";
    } else {
      image_->background_color_index_ = background_color;
    }
  }
  image_->color_resolution_ = packed & 0x70 >> 4;
  return Result::Ok();
}

Result GifImage::Parser::ParseGlobalColorTable() {
  auto result = BuildColorTable(image_->global_color_table_.get());
  if (!result.ok())
    return result;

  handler_ = HANDLER(ParseBlockType);
  return Result::Ok();
}

Result GifImage::Parser::ParseBlockType() {
  uint8_t* data;
  auto result = source_->ReadN(&data, 1);
  LOG(INFO) << "ParseBlockType " << static_cast<uint32_t>(*data);
  if (!result.ok()) {
    if (!image_->frames_.empty() && result.eof()) {
      // End of block is valid EOF if at least one frame present.
      handler_ = FinishParse;
      complete_ = true;
      return Result::Ok();
    }
    return Result::FromIoResult(result, false);
  }
  auto block_type = static_cast<char>(*data);
  if (block_type == '!') {
    handler_ = HANDLER(ParseExtensionType);
  } else if (block_type == ',') {
    handler_ = HANDLER(ParseImageDescriptor);
  } else if (block_type == ';') {
    // End of image.
    handler_ = FinishParse;
    complete_ = true;
  } else {
    // From blink GIFImageReader.cpp:
    //
    // If we get anything other than ',' (image separator), '!'
    // (extension), or ';' (trailer), there is extraneous data
    // between blocks. The GIF87a spec tells us to keep reading
    // until we find an image separator, but GIF89a says such
    // a file is corrupt. We follow Mozilla's implementation and
    // proceed as if the file were correctly terminated, so the
    // GIF will display.
    handler_ = FinishParse;
    complete_ = true;
    LOG(WARNING) << "Corrupt GIF format";
  }
  return Result::Ok();
}

Result GifImage::Parser::ParseExtensionType() {
  uint8_t* data;
  auto result = source_->ReadN(&data, 1);
  if (!result.ok())
    return Result::FromIoResult(result, false);
  switch (*data) {
    case 0xF9:
      handler_ = BlockHandler(HANDLER(ParseControlExtension));
      break;
    // From blink GIFImageReader.cpp:
    //
    // The GIF spec also specifies the lengths of the following two extensions'
    // headers (as 12 and 11 bytes, respectively). Because we ignore the plain
    // text extension entirely and sanity-check the actual length of the
    // application extension header before reading it, we allow GIFs to deviate
    // from these values in either direction. This is important for real-world
    // compatibility, as GIFs in the wild exist with application extension
    // headers that are both shorter and longer than 11 bytes.
    case 0x01:
      // Ignore plain text extension.
      handler_ = BlockHandler(HANDLER(SkipBlock));
      break;
    case 0xff:
      handler_ = BlockHandler(HANDLER(ParseApplicationExtension));
      break;
    case 0xfe:
    // Ignore comments.
    default:
      handler_ = BlockHandler(HANDLER(SkipBlock));
      break;
  }
  return Result::Ok();
}

Result GifImage::Parser::ParseSubBlockLength(Handler block_handler) {
  // First, read block length.
  uint8_t* data;
  auto io_result = source_->ReadN(&data, 1);
  if (!io_result.ok())
    return Result::FromIoResult(io_result, false);
  remaining_block_length_ = *data;
  if (remaining_block_length_ == 0) {
    LOG(INFO) << "Block ended";
    if (end_of_block_handler_) {
      auto result = end_of_block_handler_();
      end_of_block_handler_ = Handler();
      start_of_subblock_handler_ = Handler();
      if (!result.ok())
        return result;
    }
    // 0-length means end of the block. Start reading new one.
    handler_ = HANDLER(ParseBlockType);
  } else {
    LOG(INFO) << "New subblock " << remaining_block_length_;
    if (start_of_subblock_handler_) {
      auto result = start_of_subblock_handler_();
      if (!result.ok())
        return result;
    }
    // Otherwise, set up provided |block_handler|.
    handler_ = block_handler;
  }
  return Result::Ok();
}

Result GifImage::Parser::ParseControlExtension() {
  const size_t kLength = 4;
  // The GIF spec mandates that the GIFControlExtension header block length is 4
  // bytes, and the parser for this block reads 4 bytes, so we must enforce that
  // the buffer contains at least this many bytes. If the GIF specifies a
  // different length, we allow that, so long as it's larger; the additional
  // data will simply be ignored.
  if (remaining_block_length_ < kLength) {
    return Result::Error(
        Result::Code::kDecodeError,
        "Graphics Control Extension MUST be at least 4 bytes long");
  }

  LOG(INFO) << "ParseControlExtension";

  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);

  auto* builder = GetFrameParser();
  Reader reader(data);
  auto packed = reader.ReadByte();
  int disposal_method = (packed >> 2) & 0x07;
  if (disposal_method < 4) {
    // NOTE: This relies on the values in the GifImage::DisposalMethod enum
    // matching those in the GIF spec!
    builder->SetDisposalMethod(static_cast<DisposalMethod>(disposal_method));
  } else if (disposal_method == 4) {
    builder->SetDisposalMethod(DisposalMethod::kOverwritePrevious);
  }
  auto frame_duration = reader.ReadUint16();

  // |frame_duration| is the number of hundredths of a second to wait before
  // moving on to the next frame.
  // Thus multiply by 10 to get milliseconds.
  builder->SetDuration(frame_duration * 10);

  if (packed & 0x01) {
    size_t transparency_pixel = reader.ReadByte();
    builder->SetTransparentPixel(transparency_pixel);
  }

  remaining_block_length_ -= kLength;

  LOG(INFO) << "Control Ended: " << remaining_block_length_;

  return SkipSubBlockHandler(HANDLER(SkipBlock));
}

Result GifImage::Parser::ParseApplicationExtension() {
  const size_t kLength = 11;
  if (remaining_block_length_ < kLength) {
    LOG(WARNING)
        << "Application Extension header MUST be 11 bytes long, skipping block";
    handler_ = HANDLER(SkipBlock);
    return Result::Ok();
  }

  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);

  if (std::memcmp(data, "NETSCAPE2.0", kLength) ||
      std::memcmp(data, "ANIMEXTS1.0", kLength)) {
    handler_ = BlockHandler(HANDLER(ParseNetscapeApplicationExtension));
  } else if (std::memcmp(data, "ICCRGBG1012", kLength) &&
             !image_->metadata_.Has(ImageMetadata::Type::kICC)) {
    // Store only first metadata chunk of each type.
    LOG(INFO) << "ICC";
    SetupHandlers(
        [this]() -> Result {
          if (!metadata_writer_)
            metadata_writer_ =
                base::make_unique<io::BufferWriter>(remaining_block_length_);
          return Result::Ok();
        },
        [this]() -> Result { return ConsumeMetadata(); },
        [this]() -> Result {
          for (auto& chunk : metadata_writer_->ReleaseChunks())
            image_->metadata_.Append(ImageMetadata::Type::kICC,
                                     std::move(chunk));

          metadata_writer_.reset();
          return Result::Ok();
        });
  } else if (std::memcmp(data, "XMP DataXMP", kLength) &&
             !image_->metadata_.Has(ImageMetadata::Type::kXMP)) {
    // Store only first metadata chunk of each type.
    LOG(INFO) << "XMP";
    SetupHandlers(
        [this]() -> Result {
          if (!metadata_writer_)
            metadata_writer_ =
                base::make_unique<io::BufferWriter>(remaining_block_length_);

          // Special case for XMP data: In each sub-block, the first byte
          // is also part of the XMP payload. XMP in GIF also has a 257
          // byte padding data. See the XMP specification for details.
          source_->UnreadN(1);
          uint8_t* first_byte;
          auto result = source_->ReadN(&first_byte, 1);
          DCHECK(result.ok());
          metadata_writer_->Write(first_byte, 1);
          return Result::Ok();
        },
        [this]() -> Result { return ConsumeMetadata(); },
        [this]() -> Result {
          const size_t kXMPMagicTrailerSize = 257;
          if (metadata_writer_->total_size() > kXMPMagicTrailerSize)
            metadata_writer_->UnwriteN(kXMPMagicTrailerSize);

          for (auto& chunk : metadata_writer_->ReleaseChunks())
            image_->metadata_.Append(ImageMetadata::Type::kXMP,
                                     std::move(chunk));

          metadata_writer_.reset();
          return Result::Ok();
        });
  } else {
    LOG(WARNING) << "Unsupported Application Extension";
    handler_ = BlockHandler(HANDLER(SkipBlock));
    return Result::Ok();
  }

  handler_ = FinishParse;
  return Result::Ok();
}

Result GifImage::Parser::ParseNetscapeApplicationExtension() {
  const size_t kLength = 3;
  if (remaining_block_length_ < kLength) {
    LOG(WARNING)
        << "Netscape Extension MUST be at least 3 bytes long, skipping";
    handler_ = HANDLER(SkipBlock);
    return Result::Ok();
  }

  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);

  Reader reader(data);
  int netscape_extension = reader.ReadByte() & 0x07;
  if (netscape_extension == 1) {
    auto loop_count = reader.ReadUint16();
    if (loop_count == 0) {
      image_->loop_count_ = kInifiniteLoop;
    } else {
      image_->loop_count_ = loop_count;
    }
  } else if (netscape_extension == 2) {
    // From blink GIFImageReader.cpp:
    // Wait for specified # of bytes to enter buffer.

    // Don't do this, this extension doesn't exist (isn't used at all)
    // and doesn't do anything, as streaming/buffering takes care of it all...
    // See: http://semmix.pl/color/exgraf/eeg24.htm
  } else {
    LOG(ERROR) << "Unknown netscape extension: " << netscape_extension;
    return Result::Error(Result::Code::kDecodeError,
                         "Unknown netscape extension");
  }

  return SkipSubBlockHandler(HANDLER(ParseNetscapeApplicationExtension));
}

Result GifImage::Parser::ConsumeMetadata() {
  DCHECK(metadata_writer_);
  while (remaining_block_length_ > 0) {
    uint8_t* data;
    auto result = source_->ReadAtMostN(&data, remaining_block_length_);
    if (!result.ok())
      return Result::FromIoResult(result, false);

    metadata_writer_->Write(data, result.n());
    remaining_block_length_ -= result.n();
  }

  handler_ = BlockHandler(HANDLER(ConsumeMetadata));
  return Result::Ok();
}

Result GifImage::Parser::SkipSubBlockHandler(Handler handler) {
  // Consume the remaining sub-block, if anything left.
  while (remaining_block_length_ > 0) {
    uint8_t* nothing;
    auto result = source_->ReadAtMostN(&nothing, remaining_block_length_);
    if (!result.ok())
      return Result::FromIoResult(result, false);

    remaining_block_length_ -= result.n();
  }

  // Setup new sub-block handler.
  handler_ = BlockHandler(handler);
  return Result::Ok();
}

Result GifImage::Parser::SkipBlock() {
  return SkipSubBlockHandler(HANDLER(SkipBlock));
}

Result GifImage::Parser::ParseImageDescriptor() {
  LOG(INFO) << "ParseImageDescriptor";
  const size_t kLength = 9;
  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);

  Reader reader(data);
  auto x_offset = reader.ReadUint16();
  auto y_offset = reader.ReadUint16();
  auto width = reader.ReadUint16();
  auto height = reader.ReadUint16();

  auto* builder = GetFrameParser();
  builder->SetGlobalColorTable(image_->global_color_table_.get());

  // From blink GIFImageReader.cpp:
  //
  // Some GIF files have frames that don't fit in the specified
  // overall image size. For the first frame, we can simply enlarge
  // the image size to allow the frame to be visible.  We can't do
  // this on subsequent frames because the rest of the decoding
  // infrastructure assumes the image size won't change as we
  // continue decoding, so any subsequent frames that are even
  // larger will be cropped.
  // Luckily, handling just the first frame is sufficient to deal
  // with most cases, e.g. ones where the image size is erroneously
  // set to zero, since usually the first frame completely fills
  // the image.
  if (image_->frames_.empty()) {
    auto frame_width = static_cast<uint16_t>(
        std::min(static_cast<int>(std::numeric_limits<uint16_t>::max()),
                 x_offset + width));
    auto frame_height = static_cast<uint16_t>(
        std::min(static_cast<int>(std::numeric_limits<uint16_t>::max()),
                 y_offset + height));
    image_->screen_width_ = std::max(image_->screen_width_, frame_width);
    image_->screen_height_ = std::max(image_->screen_height_, frame_height);
  }

  // From blink GIFImageReader.cpp:
  //
  // Work around more broken GIF files that have zero image width or
  // height.
  if (!height || !width) {
    height = image_->screen_width_;
    width = image_->screen_height_;
    if (!height || !width)
      return Result::Error(Result::Code::kDecodeError,
                           "Invalid image width/height");
  }

  builder->SetFrameGeometry(x_offset, y_offset, width, height);

  auto packed = reader.ReadByte();

  if (packed & 0x40)
    builder->SetProgressive(true);

  if (packed & 0x80) {
    builder->CreateLocalColorTable(1 << ((packed & 0x07) + 1));
    handler_ = HANDLER(ParseLocalColorTable);
  } else {
    handler_ = HANDLER(ParseMinimumCodeSize);
  }

  header_complete_ = true;
  if (header_only_)
    return Result::Finish(Result::Code::kOk);

  return Result::Ok();
}

Result GifImage::Parser::ParseLocalColorTable() {
  auto result = BuildColorTable(GetFrameParser()->GetLocalColorTable());
  if (!result.ok())
    return result;

  handler_ = HANDLER(ParseMinimumCodeSize);
  return Result::Ok();
}

Result GifImage::Parser::ReadLZWData() {
  while (remaining_block_length_ > 0) {
    uint8_t* data;
    auto io_result = source_->ReadAtMostN(&data, remaining_block_length_);
    if (!io_result.ok())
      return Result::FromIoResult(io_result, false);

    LOG(INFO) << "ReadLZWData: " << io_result.n();

    auto result = GetFrameParser()->ProcessImageData(data, io_result.n());
    if (!result.ok())
      return result;

    remaining_block_length_ -= io_result.n();
  }

  handler_ = BlockHandler(HANDLER(ReadLZWData));
  return Result::Ok();
}

Result GifImage::Parser::ParseMinimumCodeSize() {
  const size_t kLength = 1;
  uint8_t* data;
  auto result = source_->ReadN(&data, kLength);
  if (!result.ok())
    return Result::FromIoResult(result, false);
  LOG(INFO) << "ParseMinimumCodeSize " << static_cast<uint32_t>(*data);
  if (!GetFrameParser()->InitDecoder(*data))
    return Result::Error(Result::Code::kDecodeError,
                         "Too big minimum code size");

  SetupHandlers(Handler(), [this]() -> Result { return ReadLZWData(); },
                [this]() -> Result {
                  auto frame = GetFrameParser()->ReleaseFrame();
                  active_frame_builder_.reset();
                  image_->frames_.push_back(std::move(frame));
                  return Result::Ok();
                });

  return Result::Ok();
}

}  // namespace image
