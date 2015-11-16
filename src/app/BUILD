cc_library(
  name = "image_optimizer_service",
  includes = [
    "../third_party/glog/exported/src",
  ],
  hdrs = [
    "image_optimizer_service.h",
  ],
  srcs = [
    "image_optimizer_service.cc"
  ],
  deps = [
    "//external:grpc++",
    "//image:image",
    "//io:io",
    "//proto:image_optimizer_cc",
    "//third_party/glog:glog",
  ],
  visibility = ["//visibility:public"]
)

cc_binary(
  name = "server",
  srcs = [
    "server.cc",
  ],
  includes = [
    "../third_party/glog/exported/src",
    "../third_party/gflags/exported/include",
  ],
  copts = [
    "-Iexternal/libwebp/src",
  ],
  deps = [
    "@giflib//:dgiflib",
    "@giflib//:egiflib",
    "@libpng//:libpng",
    "@libwebp//:libwebp",
    "@libwebp//:libwebp_enc_mux",
    "//third_party/gflags:gflags",
    ":image_optimizer_service",
  ],
)

cc_binary(
  name = "client",
  srcs = [
    "client.cc",
  ],
  includes = [
    "../third_party/glog/exported/src",
    "../third_party/gflags/exported/include",
  ],
  deps = [
    "//external:grpc++",
    "//third_party/gflags:gflags",
    "//third_party/glog:glog",
    "//proto:image_optimizer_cc",
  ],
)

cc_test(
  name = "end_to_end_test",
  includes = [
    "../external/googletest/googletest/include",
  ],
  srcs = [
    "optimizer_end_to_end_test.cc"
  ],
  deps = [
    "@googletest//:googletest",
    ":image_optimizer_service",
  ],
)

cc_test(
  name = "my_test",
  srcs = ["my_test.cc"],
  copts = [
    "-Iexternal/googletest/googletest/include",
    "-Iexternal/googletest/googlemock/include",
  ],
  deps = [
    "@googletest//:googletest",
    "@googletest//:googlemock",
  ],
)
