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

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD/MIT-like license (for zlib)

cc_library(
  name = "z",
  srcs = [
    "upstream/adler32.c",
    "upstream/compress.c",
    "upstream/crc32.c",
    "upstream/deflate.c",
    "upstream/gzclose.c",
    "upstream/gzlib.c",
    "upstream/gzread.c",
    "upstream/gzwrite.c",
    "upstream/infback.c",
    "upstream/inffast.c",
    "upstream/inflate.c",
    "upstream/inftrees.c",
    "upstream/trees.c",
    "upstream/uncompr.c",
    "upstream/zutil.c",
  ],
  hdrs = [
    "upstream/crc32.h",
    "upstream/deflate.h",
    "upstream/gzguts.h",
    "upstream/inffast.h",
    "upstream/inffixed.h",
    "upstream/inflate.h",
    "upstream/inftrees.h",
    "upstream/trees.h",
    "upstream/zconf.h",
    "upstream/zlib.h",
    "upstream/zutil.h",
  ],
  copts = [
    "-Wno-unused-variable",
    "-Wno-implicit-function-declaration",
  ],
)
