# Copyright 2016 Alexey Baranov <me@kotiki.cc>. All rights reserved.
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

#load("/tools/build_defs/docker/docker", "docker_build")
load("/tools/build_defs/pkg/pkg", "pkg_tar", "pkg_deb")

genrule(
  name = "rename_squim",
  srcs = ["//app:server"],
  outs = ["squimd"],
  cmd = "cp $< $@",
)

genrule(
  name = "rename_squim_conf",
  srcs = ["//release:squim.conf"],
  outs = ["etc/squim/squim.conf"],
  cmd = "cp $< $@",
)

pkg_tar(
  name = "squim_bin",
  files = [":squimd"],
  mode = "0755",
  package_dir = "/usr/bin",
  strip_prefix = ".",
)

pkg_tar(
  name = "squim_conf",
  files = [":etc/squim/squim.conf"],
  mode = "0644",
  strip_prefix = ".",
)

pkg_tar(
  name = "debian_data",
  extension = "tar.gz",
  deps = [
    ":squim_bin",
    ":squim_conf",
  ],
)

pkg_deb(
  name = "squim_debian",
  architecture = "amd64",
  built_using = "bazel (0.1.4)",
  data = ":debian_data",
  depends = [
    "libc6 (>= 2.14)",
    "libgcc1 (>= 4.9)",
    "libstdc++6 (>= 4.8)",
  ],
  description_file = "debian/description",
  postinst = "debian/postinst",
  homepage = "https://github.com/baranov1ch/squim",
  maintainer = "Alexey Baranov <me@kotiki.cc>",
  package = "squim-server",
  version = "0.0.1",
)

#docker_build(
#  name = "docker",
#  data = [
#    "squim.conf",
#    "squim.sh",
#    "//app:server",
#  ],
#  image_name = "squim/server",
#  use_cache = True,
#)
