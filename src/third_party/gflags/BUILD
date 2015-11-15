licenses(["notice"])

load("/tools/build_rules/gen_headers", "gen_headers")

gen_headers(
  name = "configure",
  files = [
    "src/config",
    "src/gflags",
    "src/gflags_completions",
    "src/gflags_declare",
  ],
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
  },
)

cc_library(
  name = "gflags",
  linkstatic = 1,
  hdrs = [
    ":src/gflags.h",
    ":src/config.h",
    ":src/gflags_completions.h",
    ":src/gflags_declare.h",
    "src/mutex.h",
    "src/util.h",
  ],
  srcs = [
    "src/gflags.cc",
    "src/gflags_reporting.cc",
    "src/gflags_completions.cc",
  ],
  includes = [
    "src"
  ],
  copts = [
    "-Wno-sign-compare",
    "-Wno-format",
  ],
  visibility = ["//visibility:public"]
)
