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

#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "squim/app/image_optimizer_service.h"
#include "squim/app/optimization.h"
#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"

DEFINE_string(listen, "0.0.0.0:50051", "address to listen on");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);

  ImageOptimizerService service(base::make_unique<WebPOptimization>());
  grpc::ServerBuilder builder;
  builder.AddListeningPort(FLAGS_listen, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  auto server = builder.BuildAndStart();
  LOG(INFO) << "Server listening on " << FLAGS_listen;
  server->Wait();
  return 0;
}
