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

load("/tools/build_rules/yasm", "yasm_library")

licenses(["notice"])

yasm_library(
  name = "simd",
  defines = [
    "-DRGBX_FILLER_0XFF",
    "-DSTRICT_MEMORY_ACCESS",
  ],
  includes = [
    ".",
    "upstream/simd",
  ],
  srcs = [
    "upstream/simd/jccolor-sse2-64.asm",
    "upstream/simd/jcgray-sse2-64.asm",
    "upstream/simd/jcsample-sse2-64.asm",
    "upstream/simd/jdcolor-sse2-64.asm",
    "upstream/simd/jdmerge-sse2-64.asm",
    "upstream/simd/jdsample-sse2-64.asm",
    "upstream/simd/jfdctflt-sse-64.asm",
    "upstream/simd/jfdctfst-sse2-64.asm",
    "upstream/simd/jfdctint-sse2-64.asm",
    "upstream/simd/jidctflt-sse2-64.asm",
    "upstream/simd/jidctfst-sse2-64.asm",
    "upstream/simd/jidctint-sse2-64.asm",
    "upstream/simd/jidctred-sse2-64.asm",
    "upstream/simd/jquantf-sse2-64.asm",
    "upstream/simd/jquanti-sse2-64.asm",
  ],
  deps = [
    "upstream/simd/jccolext-sse2-64.asm",
    "upstream/simd/jcgryext-sse2-64.asm",
    "upstream/simd/jcolsamp.inc",
    "upstream/simd/jdcolext-sse2-64.asm",
    "upstream/simd/jdct.inc",
    "upstream/simd/jdmrgext-sse2-64.asm",
    "upstream/simd/jsimdext.inc",
    "jsimdcfg.inc",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "jpeg_turbo",
  includes = ["."],
  hdrs = [
    "config.h",
    "upstream/jccolext.c",
    "upstream/jdcol565.c",
    "upstream/jdcolext.c",
    "upstream/jdmrg565.c",
    "upstream/jdmrgext.c",
    "upstream/jpeglib.h",
    "upstream/jsimd.h",
    "upstream/jstdhuff.c",
    "upstream/simd/jsimd.h",
  ],
  srcs = [
    "jconfig.h",
    "jconfigint.h",
    "upstream/jaricom.c",
    "upstream/jcapimin.c",
    "upstream/jcapistd.c",
    "upstream/jcarith.c",
    "upstream/jccoefct.c",
    "upstream/jccolor.c",
    "upstream/jcdctmgr.c",
    "upstream/jchuff.c",
    "upstream/jchuff.h",
    "upstream/jcinit.c",
    "upstream/jcmainct.c",
    "upstream/jcmarker.c",
    "upstream/jcmaster.c",
    "upstream/jcomapi.c",
    "upstream/jcparam.c",
    "upstream/jcphuff.c",
    "upstream/jcprepct.c",
    "upstream/jcsample.c",
    "upstream/jctrans.c",
    "upstream/jdapimin.c",
    "upstream/jdapistd.c",
    "upstream/jdarith.c",
    "upstream/jdatadst.c",
    "upstream/jdatasrc.c",
    "upstream/jdcoefct.c",
    "upstream/jdcolor.c",
    "upstream/jdct.h",
    "upstream/jddctmgr.c",
    "upstream/jdhuff.c",
    "upstream/jdhuff.h",
    "upstream/jdinput.c",
    "upstream/jdmainct.c",
    "upstream/jdmarker.c",
    "upstream/jdmaster.c",
    "upstream/jdmerge.c",
    "upstream/jdphuff.c",
    "upstream/jdpostct.c",
    "upstream/jdsample.c",
    "upstream/jdtrans.c",
    "upstream/jerror.c",
    "upstream/jerror.h",
    "upstream/jfdctflt.c",
    "upstream/jfdctfst.c",
    "upstream/jfdctint.c",
    "upstream/jidctflt.c",
    "upstream/jidctfst.c",
    "upstream/jidctint.c",
    "upstream/jidctred.c",
    "upstream/jinclude.h",
    "upstream/jmemmgr.c",
    "upstream/jmemnobs.c",
    "upstream/jmemsys.h",
    "upstream/jmorecfg.h",
    "upstream/jpegcomp.h",
    "upstream/jpegint.h",
    "upstream/jpeg_nbits_table.h",
    "upstream/jquant1.c",
    "upstream/jquant2.c",
    "upstream/jsimddct.h",
    "upstream/simd/jsimd_x86_64.c",
    "upstream/jutils.c",
    "upstream/jversion.h",
  ],
  deps = [":simd"],
  visibility = ["//visibility:public"],
)
