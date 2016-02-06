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

load("/tools/build_rules/gen_config_header", "gen_config_header")

replacements = {
  "#cmakedefine OS_WINDOWS": "",
  "#cmakedefine HAVE_STDINT_H": "#define HAVE_STDINT_H 1",
  "#cmakedefine HAVE_SYS_TYPES_H": "",
  "#cmakedefine HAVE_INTTYPES_H": "",
  "#cmakedefine HAVE_SYS_STAT_H": "#define HAVE_SYS_STAT_H 1",
  "#cmakedefine HAVE_UNISTD_H": "#define HAVE_UNISTD_H 1",
  "#cmakedefine HAVE_FNMATCH_H": "#define HAVE_FNMATCH_H 1",
  "#cmakedefine HAVE_SHLWAPI_H": "",
  "#cmakedefine HAVE_STRTOLL": "#define HAVE_STRTOLL 1",
  "#cmakedefine HAVE_STRTOQ": "",
  "#cmakedefine HAVE_PTHREAD": "#define HAVE_PTHREAD 1",
  "#cmakedefine HAVE_RWLOCK": "",
  "@PROJECT_NAME@": "\"gflags\"",
  "@PACKAGE_NAME@": "\"gflags\"",
  "@PACKAGE_VERSION@": "\"2.2.0\"",
  "@PACKAGE_STRING@": "\"gflags 2.2.0\"",
  "@PACKAGE_TARNAME@": "\"gflags-2.2.0\"",
  "@PACKAGE_BUGREPORT@": "\"https://github.com/schuhschuh/gflags/issues\"",
  "@GFLAGS_NAMESPACE@": "gflags",
  "@GFLAGS_IS_A_DLL@": "GFLAGS_IS_A_DLL",
  "@HAVE_STDINT_H@": "1",
  "@HAVE_SYS_TYPES_H@": "0",
  "@HAVE_INTTYPES_H@": "0",
  "@GFLAGS_INTTYPES_FORMAT_C99@": "1",
  "@GFLAGS_INTTYPES_FORMAT_BSD@": "0",
  "@GFLAGS_INTTYPES_FORMAT_VC7@": "0",
  "@GFLAGS_ATTRIBUTE_UNUSED@": "__attribute((unused))",
  "@INCLUDE_GFLAGS_NS_H@": "",
}

gen_config_header(
  name = "config_h",
  src = "upstream/src/config.h.in",
  out = "upstream/src/config.h",
  replacements = replacements
)

gen_config_header(
  name = "gflags_h",
  src = "upstream/src/gflags.h.in",
  out = "upstream/include/gflags/gflags.h",
  replacements = replacements
)

gen_config_header(
  name = "gflags_completions_h",
  src = "upstream/src/gflags_completions.h.in",
  out = "upstream/include/gflags/gflags_completions.h",
  replacements = replacements
)

gen_config_header(
  name = "gflags_declare_h",
  src = "upstream/src/gflags_declare.h.in",
  out = "upstream/include/gflags/gflags_declare.h",
  replacements = replacements
)

cc_library(
  name = "gflags",
  linkstatic = 1,
  hdrs = [
    ":upstream/src/config.h",
    ":upstream/include/gflags/gflags.h",
    ":upstream/include/gflags/gflags_completions.h",
    ":upstream/include/gflags/gflags_declare.h",
    "upstream/src/mutex.h",
    "upstream/src/util.h",
  ],
  srcs = [
    "upstream/src/gflags.cc",
    "upstream/src/gflags_reporting.cc",
    "upstream/src/gflags_completions.cc",
  ],
  includes = [
    "upstream/src",
    "upstream/include/gflags",
  ],
  copts = [
    "-Wno-sign-compare",
    "-Wno-format",
  ],
  visibility = ["//visibility:public"]
)
