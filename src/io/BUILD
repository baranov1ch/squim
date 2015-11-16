cc_library(
  name = "io",
  hdrs = [
    "buf_reader.h",
    "buffered_source.h",
    "chunk.h",
    "io_result.h",
    "writer.h",
  ],
  srcs = [
    "buf_reader.cc",
    "buffered_source.cc",
    "chunk.cc",
    "io_result.cc",
  ],
  deps = [
    "//base:base",
    "//external:glog",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "io_test",
  timeout = "short",
  srcs = [
    "buf_reader_test.cc",
    "buffered_source_test.cc",
    "chunk_test.cc",
  ],
  copts = ["-Iexternal/googletest/googletest/include"],
  deps = [
    ":io",
    "@googletest//:googletest",
  ],
)