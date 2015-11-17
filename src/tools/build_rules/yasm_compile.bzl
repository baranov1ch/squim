def _compile_single(ctx):
  yasm_path = "yasm "
  args = []

  if ctx.attr.arch == "x64":
    args += ["-felf64", "-D__x86_64__", "-DELF"]
  else:
    args += ["-felf", "-D__x86__", "-DELF"]

  args += ctx.attr.defines

  include_paths = []
  include_paths += ["-I./" + e.path for e in ctx.files.includes]
  args += include_paths
  args += [ctx.file.src.path]
  args += ["-o ", ctx.outputs.out.path]

  command = yasm_path + " ".join(args)
  ctx.action(
    mnemonic = "YasmCompile",
    inputs = [ctx.file.src] + ctx.files.deps,
    outputs = [ctx.outputs.out],
    command = command,
  )

  return struct(files = set([ctx.outputs.out]))

_yasm_compile_attrs = {
  "src": attr.label(allow_files = FileType([".asm"]),
                    single_file = True),
  "arch": attr.string(default = "x64"),
  "includes": attr.label_list(allow_files = True),
  "deps": attr.label_list(allow_files = True),
  "defines": attr.string_list(),
}

yasm_compile = rule(
  _compile_single,
  attrs = _yasm_compile_attrs,
  outputs = {
    "out": "%{name}.o"
  }
)

def many_yasm(name, arch=None, srcs=None, deps=[], includes=[], defines=[], visibility=None):
  yasm_objs = []
  for src in srcs:
    yasm_objs += [yasm_compile(name = src[:-4],
                               arch = arch,
                               src = src,
                               deps = deps,
                               includes = includes,
                               defines = defines)]
  lablels = [e.label() for e in yasm_objs]
  native.cc_library(
    linkstatic = 1,
    name = name,
    visibility = visibility,
    srcs = lablels
  )
  


