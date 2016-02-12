# Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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
  name = "ioutil",
  hdrs = [
    "chunk_reader.h",
    "chunk_writer.h",
    "read_util.h",
    "string_reader.h",
    "string_writer.h",
  ],
  srcs = [
    "chunk_reader.cc",
    "chunk_writer.cc",
    "read_util.cc",
    "string_reader.cc",
    "string_writer.cc",
  ],
  deps = [
    "//squim/io:io",
  ],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "file_util",
  hdrs = [
    "file_util.h",
  ],
  srcs = [
    "file_util.cc",
  ],
  deps = [
    ":ioutil",
    "//squim/os:os",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "ioutil_test",
  timeout = "short",
  srcs = [
  ],
  deps = [
    "//external:gtest",
    "//squim/test:test_main",
    ":ioutil",
  ],
)