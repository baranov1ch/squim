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
  name = "base",
  hdrs = [
    "logging.h",
    "make_noncopyable.h",
    "memory/make_unique.h",
    "optional.h",
    "strings/string_piece.h",
    "strings/string_util.h",
  ],
  srcs = [
    "strings/string_piece.cc",
    "strings/string_util.cc",
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
    "//external:gtest",
    "//test:test_main",
    ":base",
  ],
)
