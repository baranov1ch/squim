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
