cc_library(
  name = "image",
  hdrs = [
    "image_constants.h",
    "image_optimizer.h",
    "optimization_strategy.h",
  ],
  srcs = [
    "image_optimizer.cc"
  ],
  deps = [
    "//base:base",
    "//external:glog",
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
    "//external:gtest",
    ":image",
  ],
)
