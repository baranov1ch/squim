#include "image_test_util.h"

#include <fstream>
#include <iterator>
#include <memory>
#include <random>
#include <string>
#include <vector>

extern "C" {
#include <setjmp.h>
}

#include "base/memory/make_unique.h"
#include "image/image_decoder.h"
#include "image/image_frame.h"
#include "image/image_info.h"
#include "image/scanline_reader.h"

extern "C" {
#include "third_party/libpng/upstream/png.h"
#include "third_party/libpng/upstream/pngstruct.h"
#include "third_party/libpng/upstream/pnginfo.h"
}

#include "gtest/gtest.h"

namespace image {

namespace {

std::string GetTestDirRoot() {
  return "image/testdata/";
}
}

// TODO: use NYI fs API.
bool ReadFile(const std::string& path, std::vector<uint8_t>* contents) {
  DCHECK(contents);
  contents->clear();
  std::fstream file(path, std::ios::in | std::ios::binary);
  if (!file.good())
    return false;

  std::vector<char> tmp((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
  contents->reserve(tmp.size());
  for (const auto& c : tmp)
    contents->push_back(static_cast<uint8_t>(c));
  return true;
}

bool ReadTestFile(const std::string& path,
                  const std::string& name,
                  const std::string& extension,
                  std::vector<uint8_t>* contents) {
  return ReadFile(GetTestDirRoot() + path + "/" + name + "." + extension,
                  contents);
}

bool ReadTestFileWithExt(const std::string& path,
                         const std::string& name_with_ext,
                         std::vector<uint8_t>* contents) {
  return ReadFile(GetTestDirRoot() + path + "/" + name_with_ext, contents);
}

bool LoadReferencePng(const std::vector<uint8_t>& png_data,
                      ImageInfo* image_info,
                      ImageFrame* image_frame) {
  DCHECK(image_info);
  DCHECK(image_frame);
  auto error_fn =
      +[](png_structp png, png_const_charp c) { longjmp(png_jmpbuf(png), 1); };

  auto warning_fn =
      +[](png_structp png, png_const_charp c) { LOG(WARNING) << c; };

  auto* png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr,
                                         error_fn, warning_fn);
  auto* info_ptr = png_create_info_struct(png_ptr);
  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return false;
  }

  struct Reader {
    const std::vector<uint8_t>& data;
    size_t offset;
  };
  Reader reader{png_data, 0};
  auto read_fn = +[](png_structp read_ptr, png_bytep data, png_size_t length) {
    auto* r = reinterpret_cast<Reader*>(png_get_io_ptr(read_ptr));
    if (r->offset + length <= r->data.size()) {
      memcpy(data, &r->data[r->offset], length);
      r->offset += length;
    }
  };
  png_set_read_fn(png_ptr, &reader, read_fn);

  auto transforms = PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_SCALE_16;
  png_read_png(png_ptr, info_ptr, transforms, nullptr);

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type, compression_type, filter_type;
  png_bytep trns = nullptr;
  int trns_count = 0;
  bool has_trns = false;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_get_tRNS(png_ptr, info_ptr, &trns, &trns_count, 0);
    has_trns = true;
  }

  image_info->width = width;
  image_info->height = height;
  image_info->type = ImageType::kPng;
  image_info->multiframe = false;
  image_info->is_progressive = interlace_type == PNG_INTERLACE_ADAM7;
  switch (color_type) {
    case PNG_COLOR_TYPE_GRAY:
      if (has_trns) {
        image_info->color_scheme = ColorScheme::kGrayScaleAlpha;
      } else {
        image_info->color_scheme = ColorScheme::kGrayScale;
      }
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      image_info->color_scheme = ColorScheme::kGrayScaleAlpha;
      break;
    case PNG_COLOR_TYPE_PALETTE:
    case PNG_COLOR_TYPE_RGB:
      if (has_trns) {
        image_info->color_scheme = ColorScheme::kRGBA;
      } else {
        image_info->color_scheme = ColorScheme::kRGB;
      }
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      image_info->color_scheme = ColorScheme::kRGBA;
      break;
  }

  image_frame->Init(image_info->width, image_info->height,
                    image_info->color_scheme);
  ScanlineReader scanlines(image_frame);
  int i = 0;
  for (auto it = scanlines.begin(); it != scanlines.end(); ++it, ++i) {
    it->WritePixels(info_ptr->row_pointers[i]);
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
  return true;
}

void CheckImageInfo(const std::string& image_file,
                    const ImageInfo& ref,
                    ImageDecoder* decoder) {
  EXPECT_TRUE(decoder->IsImageInfoComplete()) << image_file;
  EXPECT_EQ(ref.width, decoder->GetWidth()) << image_file;
  EXPECT_EQ(ref.height, decoder->GetHeight()) << image_file;
  EXPECT_EQ(ref.color_scheme, decoder->GetColorScheme()) << image_file;
}

void CheckImageFrame(const std::string& image_file,
                     ImageFrame* ref,
                     ImageDecoder* decoder) {
  ASSERT_TRUE(decoder->IsFrameCompleteAtIndex(0)) << image_file;
  auto* frame = decoder->GetFrameAtIndex(0);
  EXPECT_EQ(ref->bpp(), frame->bpp()) << image_file;
  EXPECT_EQ(ref->has_alpha(), frame->has_alpha()) << image_file;
  ScanlineReader ref_reader(ref);
  ScanlineReader test_reader(frame);
  EXPECT_EQ(ref_reader.size(), test_reader.size()) << image_file;
  for (uint32_t y = 0; y < ref->width(); ++y) {
    for (uint32_t x = 0; x < ref->height(); ++x) {
      auto* pixel = frame->GetPixel(x, y);
      auto* ref_pixel = ref->GetPixel(x, y);
      EXPECT_EQ(0, std::memcmp(pixel, ref_pixel, frame->bpp()))
          << "x: " << x << " y: " << y << " @ " << image_file;
    }
  }
}

std::vector<std::vector<size_t>> GenerateFuzzyReads(size_t total_size,
                                                    size_t max_chunk_size) {
  std::vector<std::vector<size_t>> res;
  if (max_chunk_size == 0) {
    res.push_back(std::vector<size_t>{{total_size}});
    return res;
  }

  std::random_device rd;
  std::default_random_engine e1(rd());
  std::uniform_int_distribution<int> chunk_size_dist(1, max_chunk_size);
  std::uniform_int_distribution<int> chunk_num_dist(1, 3);

  while (total_size > 0) {
    size_t num_chunks = chunk_num_dist(e1);
    std::vector<size_t> read_portion;
    for (size_t i = 0; i < num_chunks && total_size > 0; ++i) {
      size_t size = chunk_size_dist(e1);
      auto effective_size = std::min(total_size, size);
      read_portion.push_back(effective_size);
      total_size -= effective_size;
    }
    res.push_back(read_portion);
  }
  return res;
}

void ValidateDecodeWithReadSpec(
    const std::string& filename,
    const std::vector<uint8_t>& raw_data,
    DecoderBuilder decoder_builder,
    RefReader ref_reader,
    const std::vector<std::vector<size_t>>& read_spec,
    ReadType read_type) {
  ImageFrame ref_frame;
  ImageInfo ref_info;
  ASSERT_TRUE(ref_reader(raw_data, &ref_info, &ref_frame)) << filename;
  auto source =
      base::make_unique<io::BufReader>(base::make_unique<io::BufferedSource>());
  auto* source_raw = source.get();
  auto testee = decoder_builder(std::move(source));
  size_t offset = 0;
  const uint8_t* data = &raw_data[0];
  bool header_read = false;
  for (auto read_slices : read_spec) {
    for (auto chunk_size : read_slices) {
      source_raw->source()->AddChunk(
          base::make_unique<io::Chunk>(data + offset, chunk_size));
      offset += chunk_size;
    }

    if (read_type != ReadType::kReadAll && !header_read) {
      auto result = testee->DecodeImageInfo();
      EXPECT_TRUE(result.pending() || result.ok()) << filename;
      if (result.ok()) {
        header_read = true;
        CheckImageInfo(filename, ref_info, testee.get());
        if (read_type == ReadType::kReadHeaderOnly) {
          EXPECT_FALSE(testee->IsFrameCompleteAtIndex(0));
          return;
        }
      }
    } else {
      auto result = testee->Decode();
      if (offset < raw_data.size()) {
        EXPECT_EQ(Result::Code::kPending, result.code()) << filename;
      } else {
        EXPECT_EQ(Result::Code::kOk, result.code()) << filename;
      }
    }
  }
  source_raw->source()->SendEof();
  // Nothing should happen here.
  EXPECT_TRUE(testee->Decode().ok());

  if (!header_read)
    CheckImageInfo(filename, ref_info, testee.get());

  CheckImageFrame(filename, &ref_frame, testee.get());
}

}  // namespace image