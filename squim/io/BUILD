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
  name = "io",
  hdrs = [
    "buf_reader.h",
    "buf_writer.h",
    "buffer_writer.h",
    "buffered_source.h",
    "chunk.h",
    "closer.h",
    "flusher.h",
    "io_result.h",
    "reader.h",
    "seeker.h",
    "writer.h",
  ],
  srcs = [
    "buf_reader.cc",
    "buf_writer.cc",
    "buffer_writer.cc",
    "buffered_source.cc",
    "chunk.cc",
    "io_result.cc",
    "writer.cc",
  ],
  deps = [
    "//squim/base:base",
  ],
  visibility = ["//visibility:public"]
)

cc_test(
  name = "io_test",
  timeout = "short",
  srcs = [
    "buf_reader_test.cc",
    "buf_writer_test.cc",
    "buffer_writer_test.cc",
    "buffered_source_test.cc",
    "chunk_test.cc",
  ],
  deps = [
    "//external:gtest",
    "//squim/test:test_main",
    ":io",
  ],
)