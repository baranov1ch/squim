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
  name = "image_optimizer_service",
  hdrs = [
    "image_optimizer_service.h",
    "optimization.h",
  ],
  srcs = [
    "image_optimizer_service.cc",
    "optimization.cc",
  ],
  deps = [
    "//external:grpc++",
    "//image:image",
    "//proto:image_optimizer_cc",
  ],
  visibility = ["//visibility:public"]
)

cc_library(
  name = "image_optimizer_client",
  hdrs = [
    "image_optimizer_client.h",
  ],
  srcs = [
    "image_optimizer_client.cc",
  ],
  deps = [
    "//base:base",
    "//external:grpc++",
    "//proto:image_optimizer_cc",
  ],
  visibility = ["//visibility:public"]
)

cc_binary(
  name = "server",
  srcs = [
    "server.cc",
  ],
  copts = [
    "-Iexternal/libwebp/src",
  ],
  deps = [
    "//external:gflags",
    ":image_optimizer_service",
  ],
)

cc_binary(
  name = "client",
  srcs = [
    "client.cc",
  ],
  deps = [
    "//external:gflags",
    ":image_optimizer_client",
  ],
)

cc_test(
  name = "end_to_end_test",
  timeout = "short",
  srcs = [
    "optimizer_end_to_end_test.cc"
  ],
  deps = [
    "//external:gtest",
    "//test:test_main",
    ":image_optimizer_client",
    ":image_optimizer_service",
  ],
  data = glob(["testdata/**"]),
)
