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

licenses(["notice"])

genrule(
  name = "copy_pnglibconf_h",
  srcs = ["upstream/scripts/pnglibconf.h.prebuilt"],
  outs = ["upstream/pnglibconf.h"],
  cmd = "cp $< $@",
)

cc_library(
  name = "png",
  includes = ["upstream"],
  hdrs = [
    "upstream/pngconf.h",
    "upstream/pngdebug.h",
    "upstream/png.h",
    "upstream/pnginfo.h",
    "upstream/pngpriv.h",
    "upstream/pngstruct.h",
    ":upstream/pnglibconf.h",
  ],
  srcs = [
    "upstream/png.c",
    "upstream/pngerror.c",
    "upstream/pngget.c",
    "upstream/pngmem.c",
    "upstream/pngpread.c",
    "upstream/pngread.c",
    "upstream/pngrio.c",
    "upstream/pngrtran.c",
    "upstream/pngrutil.c",
    "upstream/pngset.c",
    "upstream/pngtrans.c",
    "upstream/pngwio.c",
    "upstream/pngwrite.c",
    "upstream/pngwtran.c",
    "upstream/pngwutil.c",
  ],
  copts = [
    # Just to ensure we do not use system zlib.
    "-Ithird_party/zlib",
  ],
  deps = ["//external:zlib"],
  visibility = ["//visibility:public"]
)
