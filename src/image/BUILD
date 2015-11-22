cc_library(
  name = "image",
  hdrs = [
    "image_constants.h",
    "image_frame.h",
    "image_optimizer.h",
    "optimization_strategy.h",
    "result.h",
  ],
  srcs = [
    "image_optimizer.cc",
    "result.cc",
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

cc_test(
  name = "image_test",
  timeout = "short",
  srcs = [
    "image_optimizer_test.cc",
  ],
  deps = [
    "//external:gmock",
    "//external:gtest",
    "//test:test_main",
    ":image",
  ],
)
