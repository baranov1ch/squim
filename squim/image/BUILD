# Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cc_library(
  name = "image",
  hdrs = [
    "codecs/decode_params.h",
    "codecs/gif_decoder.h",
    "codecs/jpeg_decoder.h",
    "codecs/png_decoder.h",
    "codecs/webp_decoder.h",
    "codecs/webp_encoder.h",
    "decoding_reader.h",
    "image_codec_factory.h",
    "image_constants.h",
    "image_decoder.h",
    "image_encoder.h",
    "image_frame.h",
    "image_info.h",
    "image_metadata.h",
    "image_optimization_stats.h",
    "image_reader.h",
    "image_writer.h",
    "multi_frame_writer.h",
    "optimization/codec_aware_strategy.h",
    "optimization/codec_configurator.h",
    "optimization/codec_factory_with_configurator.h",
    "optimization/convert_to_webp_strategy.h",
    "optimization/default_codec_factory.h",
    "optimization/image_optimizer.h",
    "optimization/layered_adjuster.h",
    "optimization/lazy_webp_writer.h",
    "optimization/optimization_strategy.h",
    "optimization/root_strategy.h",
    "optimization/skip_metadata_reader.h",
    "optimization/strategy_builder.h",
    "pixel.h",
    "result.h",
    "scanline_reader.h",
    "single_frame_writer.h",
  ],
  srcs = [
    "codecs/gif/gif_image.cc",
    "codecs/gif/gif_image.h",
    "codecs/gif/gif_image_frame_parser.cc",
    "codecs/gif/gif_image_frame_parser.h",
    "codecs/gif/gif_image_parser.cc",
    "codecs/gif/gif_image_parser.h",
    "codecs/gif/lzw_reader.cc",
    "codecs/gif/lzw_reader.h",
    "codecs/gif/lzw_writer.cc",
    "codecs/gif/lzw_writer.h",
    "codecs/gif_decoder.cc",
    "codecs/jpeg_decoder.cc",
    "codecs/png_decoder.cc",
    "codecs/webp/multiframe_webp_encoder.cc",
    "codecs/webp/multiframe_webp_encoder.h",
    "codecs/webp/simple_webp_encoder.cc",
    "codecs/webp/simple_webp_encoder.h",
    "codecs/webp/webp_util.cc",
    "codecs/webp/webp_util.h",
    "codecs/webp_decoder.cc",
    "codecs/webp_encoder.cc",
    "decoding_reader.cc",
    "image_constants.cc",
    "image_frame.cc",
    "image_metadata.cc",
    "multi_frame_writer.cc",
    "optimization/codec_configurator.cc",
    "optimization/codec_factory_with_configurator.cc",
    "optimization/convert_to_webp_strategy.cc",
    "optimization/default_codec_factory.cc",
    "optimization/image_optimizer.cc",
    "optimization/layered_adjuster.cc",
    "optimization/lazy_webp_writer.cc",
    "optimization/root_strategy.cc",
    "optimization/skip_metadata_reader.cc",
    "result.cc",
    "single_frame_writer.cc",
  ],
  deps = [
    "//external:libjpeg",
    "//external:libpng",
    "//external:libwebp",
    "//squim/base:base",
    "//squim/io:io",
    "//squim/ioutil:ioutil",
  ],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "image_test_support",
  testonly = 1,
  hdrs = [
    "test/image_test_util.h",
    "test/mock_decoder.h",
    "test/mock_encoder.h",
    "test/mock_image_reader.h",
  ],
  srcs = [
    "test/image_test_util.cc",
  ],
  deps = [
    ":image",
    "//external:gmock",
    "//external:gtest",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "image_test",
  timeout = "short",
  srcs = [
    "codecs/gif/gif_image_parser_test.cc",
    "codecs/gif/lzw_reader_test.cc",
    "codecs/gif_decoder_test.cc",
    "codecs/jpeg_decoder_test.cc",
    "codecs/png_decoder_test.cc",
    "codecs/webp_decoder_test.cc",
    "codecs/webp_encoder_test.cc",
    "decoding_reader_test.cc",
    "optimization/convert_to_webp_strategy_test.cc",
    "optimization/image_optimizer_test.cc",
    "optimization/lazy_webp_writer_test.cc",
    "single_frame_writer_test.cc",
  ],
  deps = [
    "//external:gmock",
    "//external:gtest",
    "//squim/test:test_main",
    ":image",
    ":image_test_support",
  ],
  data = glob(["testdata/**"]),
)
