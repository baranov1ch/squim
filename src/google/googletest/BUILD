# Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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
  ) + ["upstream/googletest/src/gtest-internal-inl.h"],
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
