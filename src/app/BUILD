cc_binary(
  name = "server",
  srcs = [
    "server.cc",
  ],
  includes = [
    "../third_party/glog/exported/src",
    "../third_party/gflags/exported/include",
    "../google/grpc/include",
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
    "//google/grpc:grpc++",
    "//third_party/gflags:gflags",
    "//third_party/glog:glog",
    "//proto:optimizer_cc",
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
    "../google/grpc/include",
  ],
  deps = [
    "//google/grpc:grpc++",
    "//third_party/gflags:gflags",
    "//third_party/glog:glog",
    "//proto:optimizer_cc",
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
    "@googletest//:googlemock"
  ],
)
