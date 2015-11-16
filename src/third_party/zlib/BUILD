package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # BSD/MIT-like license (for zlib)

cc_library(
  name = "zlib",
  srcs = [
    "exported/adler32.c",
    "exported/compress.c",
    "exported/crc32.c",
    "exported/deflate.c",
    "exported/gzclose.c",
    "exported/gzlib.c",
    "exported/gzread.c",
    "exported/gzwrite.c",
    "exported/infback.c",
    "exported/inffast.c",
    "exported/inflate.c",
    "exported/inftrees.c",
    "exported/trees.c",
    "exported/uncompr.c",
    "exported/zutil.c",
  ],
  hdrs = [
    "exported/crc32.h",
    "exported/deflate.h",
    "exported/gzguts.h",
    "exported/inffast.h",
    "exported/inffixed.h",
    "exported/inflate.h",
    "exported/inftrees.h",
    "exported/trees.h",
    "exported/zconf.h",
    "exported/zlib.h",
    "exported/zutil.h",
  ],
  copts = [
    "-Wno-unused-variable",
    "-Wno-implicit-function-declaration",
  ],
)
