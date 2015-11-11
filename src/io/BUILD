cc_library(
  name = "io",
  hdrs = [
    "buf_reader.h",
    "buffered_source.h",
    "chunk.h",
    "io_result.h",
  ],
  srcs = [
    "buf_reader.cc",
    "buffered_source.cc",
    "chunk.cc",
    "io_result.cc",
  ],
  deps = ["//base:base"]
)

cc_test(
  name = "io_test",
  timeout = "short",
  srcs = [
    "buf_reader_test.cc",
    "buffered_source_test.cc",
    "chunk_test.cc",
  ],
  copts = ["-Iexternal/gtest/gtest-1.7.0/include"],
  deps = [
    ":io",
    "@gtest//:main",
  ],
)