#ifndef IMAGE_IMAGE_TEST_UTIL_H_
#define IMAGE_IMAGE_TEST_UTIL_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "io/buf_reader.h"
#include "io/buffered_source.h"

namespace image {

class ImageDecoder;
class ImageFrame;
class ImageInfo;

// Specifies how the image should be read.
enum ReadType {
  // Read everything using Decode() method.
  kReadAll,

  // Read using DecodeImageInfo() and stop after header is complete.
  kReadHeaderOnly,

  // First read header using DecodeImageInfo(), the proceed decoding.
  kReadHeaderThenBody,
};

using RefReader = std::function<bool(ImageInfo*, ImageFrame*)>;
using DecoderBuilder = std::function<std::unique_ptr<ImageDecoder>(
    std::unique_ptr<io::BufReader>)>;

bool ReadFile(const std::string& full_path, std::vector<uint8_t>* contents);

bool ReadTestFile(const std::string& path,
                  const std::string& name,
                  const std::string& extension,
                  std::vector<uint8_t>* contents);

bool ReadTestFileWithExt(const std::string& path,
                         const std::string& name_with_ext,
                         std::vector<uint8_t>* contents);

// Decodes PNG from |png_data| using high-level libpng API. Used to verify other
// images against this picture.
// |filename| is used for logging only.
bool LoadReferencePng(const std::string& filename,
                      const std::vector<uint8_t>& png_data,
                      ImageInfo* image_info,
                      ImageFrame* image_frame);

// Compares all image properties with provided |reference|.
void CheckImageInfo(const std::string& image_file,
                    const ImageInfo& reference,
                    ImageDecoder* decoder);

// Does pixel-by-pixel comparison of the decoded frame against |reference|.
void CheckImageFrame(const std::string& image_file,
                     ImageFrame* reference,
                     ImageDecoder* decoder);

// Ensures images are more or less the same, with |min_psnr|
// (google for Peak-Signal-to-Noise-Ratio).
void CheckImageFrameByPSNR(const std::string& image_file,
                           ImageFrame* reference,
                           ImageDecoder* decoder,
                           double min_psnr);

// Creates randomized read sequence to emulate data coming from the network.
// Used to test how decoders deal with io suspension.
// Generates uniformly random sizes [1..max_chunk_size] to read. These read are
// grouped by [1..3] (that's why it vector of vectors).
// See ValidateDecodeWithReadSpec description to see how it is used.
// Special case: if |max_chunk_size| == 0, a single read of |total_size| size is
// created.
std::vector<std::vector<size_t>> GenerateFuzzyReads(size_t total_size,
                                                    size_t max_chunk_size);

// Same as above, but all reads will be inone row, i.e. used as synchronous.
std::vector<std::vector<size_t>> GenerateSyncFuzzyReads(size_t total_size,
                                                        size_t max_chunk_size);

// Does all the heavy-lifting of a single frame decoder testing.
//
// |filename| is used for logging only, so that images with failed expectations
// could be easily identified.
//
// |raw_data| is the test image data.
//
// |decoder_builder| is a function used to create decoder under test. Most of
// the decoders are created from the io source which is yet to be constructed.
// Thats why we cannot pass here decoder object itself.
//
// |read_spec| is used to create and control io::BufReader with image data:
// From single group of sizes (inner vector), a sequence of synchronous reads
// from io::BufReader is generated. Then decoder spins doing from 1 to 3 reads
// until the reader pends. Repeat until EOF.
//
// |read_type| determines which part of image should be read (header, body,
// etc).
void ValidateDecodeWithReadSpec(
    const std::string& filename,
    const std::vector<uint8_t>& raw_data,
    DecoderBuilder decoder_builder,
    RefReader ref_reader,
    const std::vector<std::vector<size_t>>& read_spec,
    ReadType read_type);

}  // namespace image

#endif  // IMAGE_IMAGE_TEST_UTIL_H_
