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
  src = "exported/src/config.h.in",
  out = "exported/src/config.h",
  replacements = replacements
)

gen_config_header(
  name = "gflags_h",
  src = "exported/src/gflags.h.in",
  out = "exported/include/gflags/gflags.h",
  replacements = replacements
)

gen_config_header(
  name = "gflags_completions_h",
  src = "exported/src/gflags_completions.h.in",
  out = "exported/include/gflags/gflags_completions.h",
  replacements = replacements
)

gen_config_header(
  name = "gflags_declare_h",
  src = "exported/src/gflags_declare.h.in",
  out = "exported/include/gflags/gflags_declare.h",
  replacements = replacements
)

cc_library(
  name = "gflags",
  linkstatic = 1,
  hdrs = [
    ":exported/src/config.h",
    ":exported/include/gflags/gflags.h",
    ":exported/include/gflags/gflags_completions.h",
    ":exported/include/gflags/gflags_declare.h",
    "exported/src/mutex.h",
    "exported/src/util.h",
  ],
  srcs = [
    "exported/src/gflags.cc",
    "exported/src/gflags_reporting.cc",
    "exported/src/gflags_completions.cc",
  ],
  includes = [
    "exported/src",
    "exported/include/gflags",
  ],
  copts = [
    "-Wno-sign-compare",
    "-Wno-format",
  ],
  visibility = ["//visibility:public"]
)
