cc_library(
  name = "giflib_core",
  srcs = [
    "lib/gifalloc.c",
    "lib/gif_err.c",
  ]
)

cc_library(
  name = "dgiflib",
  srcs = [
    "lib/dgif_lib.c"
  ],
  deps = [":giflib_core"],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "egiflib",
  srcs = [
    "lib/egif_lib.c",
    "lib/gif_hash.c",
  ],
  deps = [":giflib_core"],
  visibility = ["//visibility:public"]
)
