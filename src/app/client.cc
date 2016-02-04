/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iterator>
#include <iostream>
#include <fstream>
#include <memory>

#include "app/image_optimizer_client.h"
#include "base/logging.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "io/chunk.h"
#include "os/file_system.h"
#include "os/file.h"

DEFINE_string(in, "test.png", "input image file");
DEFINE_string(out, "test.webp", "output file");
DEFINE_string(service, "localhost:50051", "service endpoint");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  auto fs = os::FileSystem::CreateDefault();
  std::unique_ptr<os::File> in;
  std::unique_ptr<os::File> out;
  auto result = fs->Open(FLAGS_in, &in);
  if (!result.ok()) {
    LOG(ERROR) << "Cannot open input file " << result.ToString();
    return 1;
  }

  result = fs->Create(FLAGS_out, &out);
  if (!result.ok()) {
    LOG(ERROR) << "Cannot open output file " << result.ToString();
    return 1;
  }

  ImageOptimizerClient client(
      grpc::CreateChannel(FLAGS_service, grpc::InsecureChannelCredentials()));
  if (!client.OptimizeImage(in.get(), 1024, out.get())) {
    LOG(ERROR) << "Optimization failed";
    return 1;
  }

  return 0;
}
