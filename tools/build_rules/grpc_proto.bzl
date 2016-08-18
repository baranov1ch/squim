# This is almost complete copy of kythe protobuf build rules:
# https://github.com/google/kythe/blob/master/tools/build_rules/go.bzl

standard_proto_path = "google/protobuf/src/"

def _genproto_impl(ctx):
  proto_src_deps = [src.proto_src for src in ctx.attr.deps]
  inputs, outputs, arguments = [ctx.file.src] + proto_src_deps, [], ["--proto_path=."]
  for src in proto_src_deps:
    if src.path.startswith(standard_proto_path):
      arguments += ["--proto_path=" + standard_proto_path]
      break

  outputs += [ctx.outputs.cc_hdr, ctx.outputs.cc_src]
  arguments += ["--cpp_out=" + ctx.configuration.genfiles_dir.path]

  if ctx.attr.has_services:
    cpp_grpc_plugin = ctx.executable._protoc_grpc_plugin_cpp
    inputs += [cpp_grpc_plugin]
    arguments += [
      "--plugin=protoc-gen-grpc=" + cpp_grpc_plugin.path,
      "--grpc_out=" + ctx.configuration.genfiles_dir.path
    ]
    outputs += [ctx.outputs.grpc_hdr, ctx.outputs.grpc_src]

  ctx.action(
      mnemonic = "GenProto",
      inputs = inputs,
      outputs = outputs,
      arguments = arguments + [ctx.file.src.path],
      executable = ctx.executable._protoc)

  return struct(files=set(outputs),
                proto_src=ctx.file.src)

_genproto_attrs = {
    "src": attr.label(
        allow_files = FileType([".proto"]),
        single_file = True,
    ),
    "deps": attr.label_list(
        allow_files = False,
        providers = ["proto_src"],
    ),
    "has_services": attr.bool(),
    "_protoc": attr.label(
        default = Label("//external:protoc"),
        executable = True,
    ),
    "_protoc_grpc_plugin_cpp":  attr.label(
      default = Label("//external:grpc_cpp_plugin"),
      executable = True,
    ),
    "gen_cc": attr.bool(),
}

def _genproto_outputs(attrs):
  outputs = {
    "cc_hdr": "%{src}.pb.h",
    "cc_src": "%{src}.pb.cc"
  }
  if attrs.has_services:
    outputs += {
      "grpc_hdr": "%{src}.grpc.pb.h",
      "grpc_src": "%{src}.grpc.pb.cc"
    }
  return outputs

genproto = rule(
  _genproto_impl,
  attrs = _genproto_attrs,
  output_to_genfiles = True,
  outputs = _genproto_outputs,
)

def proto_library(name, src=None, deps=[], visibility=None,
                  has_services=False):
  if not src:
    if name.endswith("_proto"):
      src = name[:-6] + ".proto"
    else:
      src = name + ".proto"
  proto_pkg = genproto(name=name,
                       src=src,
                       deps=deps,
                       has_services=has_services)

  cc_deps = ["//external:protobuf_clib"]
  if has_services:
    cc_deps += [
      "//external:grpc++",
      "//external:grpc++_reflection",
    ]
  for dep in deps:
    cc_deps += [dep + "_cc"]
  native.cc_library(
      name = name + "_cc",
      visibility = visibility,
      hdrs = [proto_pkg.label()],
      srcs = [proto_pkg.label()],
      defines = ["GOOGLE_PROTOBUF_NO_RTTI"],
      deps = cc_deps,
  )
