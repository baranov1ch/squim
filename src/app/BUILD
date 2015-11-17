cc_library(
  name = "image_optimizer_service",
  hdrs = [
    "image_optimizer_service.h",
  ],
  srcs = [
    "image_optimizer_service.cc"
  ],
  deps = [
    "//external:glog",
    "//external:grpc++",
    "//image:image",
    "//io:io",
    "//proto:image_optimizer_cc",
  ],
  visibility = ["//visibility:public"]
)

cc_binary(
  name = "server",
  srcs = [
    "server.cc",
  ],
  copts = [
    "-Iexternal/libwebp/src",
  ],
  deps = [
    "//external:gflags",
    ":image_optimizer_service",
  ],
)

cc_binary(
  name = "client",
  srcs = [
    "client.cc",
  ],
  deps = [
    "//external:gflags",
    "//external:glog",
    "//external:grpc++",
    "//proto:image_optimizer_cc",
  ],
)

cc_test(
  name = "end_to_end_test",
  srcs = [
    "optimizer_end_to_end_test.cc"
  ],
  deps = [
    "//external:gtest",
    "//test:test_main",
    ":image_optimizer_service",
  ],
)

cc_test(
  name = "my_test",
  srcs = [
    "my_test.cc"
  ],
  deps = [
    "//external:gmock",
    "//external:gtest",
    "//test:test_main",
  ],
)
