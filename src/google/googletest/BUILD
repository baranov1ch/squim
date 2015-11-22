cc_library(
  name = "googletest",
  includes = [
    "upstream/googletest",
    "upstream/googletest/include"
  ],
  srcs = glob(
    ["upstream/googletest/src/*.cc"],
    exclude = [
      "upstream/googletest/src/gtest-all.cc",
      "upstream/googletest/src/gtest_main.cc",
    ]
  ),
  hdrs = glob(["upstream/googletest/include/**/*.h"]),
  linkopts = ["-pthread"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "googlemock",
  includes = [
    "upstream/googlemock",
    "upstream/googlemock/include",
    "upstream/googletest",
    "upstream/googletest/include",
  ],
  srcs = glob(
    ["upstream/googlemock/src/*.cc"],
    exclude = [
      "upstream/googlemock/src/gmock-all.cc",
      "upstream/googlemock/src/gmock_main.cc",
    ]
  ),
  hdrs = glob(["upstream/googlemock/include/**/*.h"]),
  linkopts = ["-pthread"],
  deps = [":googletest"],
  visibility = ["//visibility:public"],
)
