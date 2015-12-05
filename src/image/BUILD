cc_library(
  name = "image",
  hdrs = [
    "decoding_reader.h",
    "image_codec_factory.h",
    "image_constants.h",
    "image_decoder.h",
    "image_encoder.h",
    "image_frame.h",
    "image_info.h",
    "image_metadata.h",
    "image_optimizer.h",
    "image_reader.h",
    "image_reader_writer_factory.h",
    "image_writer.h",
    "optimization_strategy.h",
    "result.h",
    "scanline_reader.h",
    "single_frame_writer.h",
  ],
  srcs = [
    "codecs/jpeg_decoder.cc",
    "codecs/jpeg_decoder.h",
    "codecs/png_decoder.cc",
    "codecs/png_decoder.h",
    "decoding_reader.cc",
    "image_constants.cc",
    "image_frame.cc",
    "image_optimizer.cc",
    "result.cc",
    "single_frame_writer.cc",
  ],
  deps = [
    "//base:base",
    "//external:glog",
    "//external:libjpeg",
    "//external:libpng",
    "//external:libwebp",
    "//io:io",
  ],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "image_test_support",
  testonly = 1,
  hdrs = [
    "image_test_util.h",
  ],
  srcs = [
    "image_test_util.cc",
  ],
  deps = [
    ":image",
    "//external:gtest",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "image_test",
  timeout = "short",
  srcs = [
    "codecs/png_decoder_test.cc",
    "decoding_reader_test.cc",
    "image_optimizer_test.cc",
  ],
  deps = [
    "//external:gmock",
    "//external:gtest",
    "//test:test_main",
    ":image",
    ":image_test_support",
  ],
  data = glob(["testdata/**"]),
)
