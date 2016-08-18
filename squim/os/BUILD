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
  name = "os",
  hdrs = [
    "dir_util.h",
    "file.h",
    "file_info.h",
    "file_mode.h",
    "file_system.h",
    "fs_result.h",
    "os_error.h",
    "path_util.h",
  ],
  srcs = [
    "dir_util.cc",
    "file.cc",
    "file_posix.h",
    "file_posix.cc",
    "file_system.cc",
    "file_system_posix.cc",
    "file_system_posix.h",
    "fs_result.cc",
    "os_error.cc",
    "posix_util.cc",
    "posix_util.h",
  ],
  deps = [
    "//squim/base:base",
    "//squim/io:io",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "os_test",
  timeout = "short",
  srcs = [
    "dir_util_test.cc",
    "file_system_posix_test.cc",
  ],
  deps = [
    "//external:gtest",
    "//squim/test:test_main",
    ":os",
  ],
  data = glob(["testdata/**"]),
)