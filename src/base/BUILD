cc_library(
  name = "base",
  hdrs = [
    "make_noncopyable.h",
    "memory/make_unique.h",
    "strings/string_piece.h",
    "strings/string_util.h",
  ],
  srcs = [
    "strings/string_piece.cc"
  ],
  deps = [
    "//external:glog",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "base_test",
  timeout = "short",
  srcs = [
    "strings/string_piece_test.cc"
  ],
  deps = [
    ":base",
    "//external:gtest",
  ],
)
