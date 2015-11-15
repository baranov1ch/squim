cc_binary(
  name = "main",
  srcs = [
    "main.cc",
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
    "//third_party/glog:glog",
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
