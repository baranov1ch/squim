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
