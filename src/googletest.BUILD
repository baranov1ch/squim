cc_library(
  name = "googletest",
  srcs = glob(
    ["googletest/src/*.cc"],
    exclude = ["googletest/src/gtest-all.cc"]
  ),
  hdrs = glob(["googletest/include/**/*.h"]),
  copts = [
    "-Iexternal/googletest/googletest",
    "-Iexternal/googletest/googletest/include"
  ],
  linkopts = ["-pthread"],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "googlemock",
  srcs = glob(
    ["googlemock/src/*.cc"],
    exclude = ["googlemock/src/gmock-all.cc"]
  ),
  hdrs = glob(["googlemock/include/**/*.h"]),
  copts = [
    "-Iexternal/googletest/googlemock",
    "-Iexternal/googletest/googlemock/include",
    "-Iexternal/googletest/googletest",
    "-Iexternal/googletest/googletest/include"
  ],
  linkopts = ["-pthread"],
  deps = [":googletest"],
  visibility = ["//visibility:public"],
)
