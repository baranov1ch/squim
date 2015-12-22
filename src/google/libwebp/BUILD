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

cc_library(
  name = "libwebp_dec",
  hdrs = [
    "upstream/src/dec/alphai.h",
    "upstream/src/dec/decode_vp8.h",
    "upstream/src/dec/vp8i.h",
    "upstream/src/dec/vp8li.h",
    "upstream/src/dec/webpi.h",
  ],
  srcs = [
    "upstream/src/dec/alpha.c",
    "upstream/src/dec/buffer.c",
    "upstream/src/dec/frame.c",
    "upstream/src/dec/idec.c",
    "upstream/src/dec/io.c",
    "upstream/src/dec/quant.c",
    "upstream/src/dec/tree.c",
    "upstream/src/dec/vp8.c",
    "upstream/src/dec/vp8l.c",
    "upstream/src/dec/webp.c",
  ],
  deps = [
    ":libwebp_dsp",
    ":libwebp_utils",
  ],
)

cc_library(
  name = "libwebp_demux",
  srcs = [
    "upstream/src/demux/demux.c"
  ],
)

cc_library(
  name = "libwebp_dsp",
  hdrs = [
    "upstream/src/dsp/dsp.h",
    "upstream/src/dsp/lossless.h",
    "upstream/src/dsp/yuv.h",
    "upstream/src/dsp/yuv_tables_sse2.h",
  ],
  srcs = [
    "upstream/src/dsp/alpha_processing.c",
    "upstream/src/dsp/alpha_processing_sse2.c",
    "upstream/src/dsp/cpu.c",
    "upstream/src/dsp/dec.c",
    "upstream/src/dsp/dec_clip_tables.c",
    "upstream/src/dsp/dec_mips32.c",
    "upstream/src/dsp/dec_sse2.c",
    "upstream/src/dsp/enc.c",
    "upstream/src/dsp/enc_avx2.c",
    "upstream/src/dsp/enc_mips32.c",
    "upstream/src/dsp/enc_sse2.c",
    "upstream/src/dsp/lossless.c",
    "upstream/src/dsp/lossless_mips32.c",
    "upstream/src/dsp/lossless_sse2.c",
    "upstream/src/dsp/upsampling.c",
    "upstream/src/dsp/upsampling_sse2.c",
    "upstream/src/dsp/yuv.c",
    "upstream/src/dsp/yuv_mips32.c",
    "upstream/src/dsp/yuv_sse2.c",
  ],
  linkopts = ["-lm"]
)

cc_library(
  name = "libwebp_enc",
  hdrs = [
    "upstream/src/enc/backward_references.h",
    "upstream/src/enc/cost.h",
    "upstream/src/enc/histogram.h",
    "upstream/src/enc/vp8enci.h",
    "upstream/src/enc/vp8li.h",
  ],
  srcs = [
    "upstream/src/enc/alpha.c",
    "upstream/src/enc/analysis.c",
    "upstream/src/enc/backward_references.c",
    "upstream/src/enc/config.c",
    "upstream/src/enc/cost.c",
    "upstream/src/enc/filter.c",
    "upstream/src/enc/frame.c",
    "upstream/src/enc/histogram.c",
    "upstream/src/enc/iterator.c",
    "upstream/src/enc/picture.c",
    "upstream/src/enc/picture_csp.c",
    "upstream/src/enc/picture_psnr.c",
    "upstream/src/enc/picture_rescale.c",
    "upstream/src/enc/picture_tools.c",
    "upstream/src/enc/quant.c",
    "upstream/src/enc/syntax.c",
    "upstream/src/enc/token.c",
    "upstream/src/enc/tree.c",
    "upstream/src/enc/vp8l.c",
    "upstream/src/enc/webpenc.c",
  ],
  deps = [
    ":libwebp_dsp",
    ":libwebp_utils",
  ],
)

cc_library(
  name = "libwebp_mux",
  hdrs = [
    "upstream/src/mux/muxi.h",
  ],
  srcs = [
    "upstream/src/mux/muxedit.c",
    "upstream/src/mux/muxinternal.c",
    "upstream/src/mux/muxread.c",
  ],
)

cc_library(
  name = "libwebp_enc_mux",
  includes = ["upstream/src"],
  hdrs = [
    "upstream/examples/gif2webp_util.h"
  ],
  srcs = [
    "upstream/examples/gif2webp_util.c"
  ],
  deps = [":libwebp_mux"],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "libwebp_utils",
  hdrs = [
    "upstream/src/utils/bit_reader.h",
    "upstream/src/utils/bit_writer.h",
    "upstream/src/utils/color_cache.h",
    "upstream/src/utils/filters.h",
    "upstream/src/utils/huffman.h",
    "upstream/src/utils/huffman_encode.h",
    "upstream/src/utils/quant_levels.h",
    "upstream/src/utils/quant_levels_dec.h",
    "upstream/src/utils/random.h",
    "upstream/src/utils/rescaler.h",
    "upstream/src/utils/thread.h",
    "upstream/src/utils/utils.h",
  ],
  srcs = [
    "upstream/src/utils/bit_reader.c",
    "upstream/src/utils/bit_reader_inl.h",
    "upstream/src/utils/bit_writer.c",
    "upstream/src/utils/color_cache.c",
    "upstream/src/utils/endian_inl.h",
    "upstream/src/utils/filters.c",
    "upstream/src/utils/huffman.c",
    "upstream/src/utils/huffman_encode.c",
    "upstream/src/utils/quant_levels.c",
    "upstream/src/utils/quant_levels_dec.c",
    "upstream/src/utils/random.c",
    "upstream/src/utils/rescaler.c",
    "upstream/src/utils/thread.c",
    "upstream/src/utils/utils.c",
  ],
)

cc_library(
  name = "libwebp",
  deps = [
    ":libwebp_dec",
    ":libwebp_demux",
    ":libwebp_dsp",
    ":libwebp_enc",
    ":libwebp_enc_mux",
    ":libwebp_mux",
    ":libwebp_utils",
  ],
  hdrs = [
    "upstream/src/webp/decode.h",
    "upstream/src/webp/demux.h",
    "upstream/src/webp/encode.h",
    "upstream/src/webp/format_constants.h",
    "upstream/src/webp/mux.h",
    "upstream/src/webp/mux_types.h",
    "upstream/src/webp/types.h",
  ],
  visibility = ["//visibility:public"]
)
