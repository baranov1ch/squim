load("/tools/build_rules/gen_config_header", "gen_config_header")

def gen_headers(name, files, replacements, visibility=None):
  for f in files:
    gen_config_header(
      name = f,
      src = f + ".h.in",
      out = f + ".h",
      replacements = replacements,
    )
