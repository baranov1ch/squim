in_files = FileType([".in"])

def _impl(ctx):
  ctx.template_action(
    template = ctx.file.src,
    substitutions = ctx.attr.replacements,
    output = ctx.outputs.out,
    executable = False,
  )

gen_config_header = rule(
  implementation = _impl,
  output_to_genfiles = True,
  attrs = {
    "src": attr.label(allow_files = in_files, single_file = True),
    "out": attr.output(mandatory = True),
    "replacements": attr.string_dict(),
  },
)
