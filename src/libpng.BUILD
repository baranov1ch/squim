genrule(
  name = "copy_pnglibconf_h",
  srcs = ["scripts/pnglibconf.h.prebuilt"],
  outs = ["pnglibconf.h"],
  cmd = "cp $< $@",
)

cc_library(
  name = "libpng",
  includes = ["."],
  hdrs = [
    "pngconf.h",
    "pngdebug.h",
    "png.h",
    "pnginfo.h",
    "pngpriv.h",
    "pngstruct.h",
    ":pnglibconf.h",
  ],
  srcs = [
    "png.c",
    "pngerror.c",
    "pngget.c",
    "pngmem.c",
    "pngpread.c",
    "pngread.c",
    "pngrio.c",
    "pngrtran.c",
    "pngrutil.c",
    "pngset.c",
    "pngtrans.c",
    "pngwio.c",
    "pngwrite.c",
    "pngwtran.c",
    "pngwutil.c",
  ],
  visibility = ["//visibility:public"]
)
