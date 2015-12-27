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

#include <memory>

#include "app/image_optimizer_service.h"
#include "app/optimization.h"
#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "webp/encode.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"

DEFINE_bool(do_nothing, true, "If true, does nothing");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  LOG(INFO) << "Hello, World! " << FLAGS_do_nothing;
  std::string server_address("0.0.0.0:50051");
  ImageOptimizerService service(base::make_unique<WebPOptimization>());
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << server_address;
  server->Wait();
  auto success = WebPEncode(nullptr, nullptr);
  if (!success)
    return -1;
  return 0;
}
