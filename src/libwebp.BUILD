cc_library(
  name = "libwebp_dec",
  hdrs = [
    "src/dec/alphai.h",
    "src/dec/decode_vp8.h",
    "src/dec/vp8i.h",
    "src/dec/vp8li.h",
    "src/dec/webpi.h",
  ],
  srcs = [
    "src/dec/alpha.c",
    "src/dec/buffer.c",
    "src/dec/frame.c",
    "src/dec/idec.c",
    "src/dec/io.c",
    "src/dec/quant.c",
    "src/dec/tree.c",
    "src/dec/vp8.c",
    "src/dec/vp8l.c",
    "src/dec/webp.c",
  ],
  deps = [
    ":libwebp_dsp",
    ":libwebp_utils",
  ],
)

cc_library(
  name = "libwebp_demux",
  srcs = [
    "src/demux/demux.c"
  ],
)

cc_library(
  name = "libwebp_dsp",
  hdrs = [
    "src/dsp/dsp.h",
    "src/dsp/lossless.h",
    "src/dsp/yuv.h",
    "src/dsp/yuv_tables_sse2.h",
  ],
  srcs = [
    "src/dsp/alpha_processing.c",
    "src/dsp/alpha_processing_sse2.c",
    "src/dsp/cpu.c",
    "src/dsp/dec.c",
    "src/dsp/dec_clip_tables.c",
    "src/dsp/dec_mips32.c",
    "src/dsp/dec_sse2.c",
    "src/dsp/enc.c",
    "src/dsp/enc_avx2.c",
    "src/dsp/enc_mips32.c",
    "src/dsp/enc_sse2.c",
    "src/dsp/lossless.c",
    "src/dsp/lossless_mips32.c",
    "src/dsp/lossless_sse2.c",
    "src/dsp/upsampling.c",
    "src/dsp/upsampling_sse2.c",
    "src/dsp/yuv.c",
    "src/dsp/yuv_mips32.c",
    "src/dsp/yuv_sse2.c",
  ],
  linkopts = ["-lm"]
)

cc_library(
  name = "libwebp_enc",
  hdrs = [
    "src/enc/backward_references.h",
    "src/enc/cost.h",
    "src/enc/histogram.h",
    "src/enc/vp8enci.h",
    "src/enc/vp8li.h",
  ],
  srcs = [
    "src/enc/alpha.c",
    "src/enc/analysis.c",
    "src/enc/backward_references.c",
    "src/enc/config.c",
    "src/enc/cost.c",
    "src/enc/filter.c",
    "src/enc/frame.c",
    "src/enc/histogram.c",
    "src/enc/iterator.c",
    "src/enc/picture.c",
    "src/enc/picture_csp.c",
    "src/enc/picture_psnr.c",
    "src/enc/picture_rescale.c",
    "src/enc/picture_tools.c",
    "src/enc/quant.c",
    "src/enc/syntax.c",
    "src/enc/token.c",
    "src/enc/tree.c",
    "src/enc/vp8l.c",
    "src/enc/webpenc.c",
  ],
  deps = [
    ":libwebp_dsp",
    ":libwebp_utils",
  ],
)

cc_library(
  name = "libwebp_mux",
  hdrs = [
    "src/mux/muxi.h",
  ],
  srcs = [
    "src/mux/muxedit.c",
    "src/mux/muxinternal.c",
    "src/mux/muxread.c",
  ],
)

cc_library(
  name = "libwebp_enc_mux",
  hdrs = [
    "examples/gif2webp_util.h"
  ],
  srcs = [
    "examples/gif2webp_util.c"
  ],
  copts = ["-Iexternal/libwebp/src"],
  deps = [":libwebp_mux"],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "libwebp_utils",
  hdrs = [
    "src/utils/bit_reader.h",
    "src/utils/bit_writer.h",
    "src/utils/color_cache.h",
    "src/utils/filters.h",
    "src/utils/huffman.h",
    "src/utils/huffman_encode.h",
    "src/utils/quant_levels.h",
    "src/utils/quant_levels_dec.h",
    "src/utils/random.h",
    "src/utils/rescaler.h",
    "src/utils/thread.h",
    "src/utils/utils.h",
  ],
  srcs = [
    "src/utils/bit_reader.c",
    "src/utils/bit_reader_inl.h",
    "src/utils/bit_writer.c",
    "src/utils/color_cache.c",
    "src/utils/endian_inl.h",
    "src/utils/filters.c",
    "src/utils/huffman.c",
    "src/utils/huffman_encode.c",
    "src/utils/quant_levels.c",
    "src/utils/quant_levels_dec.c",
    "src/utils/random.c",
    "src/utils/rescaler.c",
    "src/utils/thread.c",
    "src/utils/utils.c",
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
    "src/webp/decode.h",
    "src/webp/demux.h",
    "src/webp/encode.h",
    "src/webp/format_constants.h",
    "src/webp/mux.h",
    "src/webp/mux_types.h",
    "src/webp/types.h",
  ],
  visibility = ["//visibility:public"]
)
