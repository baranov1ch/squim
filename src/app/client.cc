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

#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "base/logging.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "proto/image_optimizer.grpc.pb.h"
#include "proto/image_optimizer.pb.h"

using squim::ImageOptimizer;
using squim::ImageRequestPart;
using squim::ImageResponsePart;

ImageRequestPart MakeImageRequestPart(const std::string& name) {
  ImageRequestPart p;
  p.set_name(name);
  return p;
}

class OptimizerClient {
 public:
  OptimizerClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(squim::ImageOptimizer::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  void OptimizeImage() {
    grpc::ClientContext context;

    std::shared_ptr<
        grpc::ClientReaderWriter<ImageRequestPart, ImageResponsePart>>
        stream(stub_->OptimizeImage(&context));

    std::thread writer([stream]() {
      auto seed = std::chrono::system_clock::now().time_since_epoch().count();
      std::default_random_engine generator(seed);
      std::uniform_int_distribution<int> delay_distribution(500, 1500);
      std::vector<ImageRequestPart> parts{
          MakeImageRequestPart("First message"),
          MakeImageRequestPart("Second message"),
          MakeImageRequestPart("Third message"),
          MakeImageRequestPart("Fourth message")};
      for (const auto& part : parts) {
        LOG(INFO) << "Sending message " << part.name();
        stream->Write(part);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(delay_distribution(generator)));
      }
      stream->WritesDone();
    });

    ImageResponsePart response_part;
    while (stream->Read(&response_part)) {
      LOG(INFO) << "Got message " << response_part.message();
    }
    writer.join();
    auto status = stream->Finish();
    if (!status.ok()) {
      LOG(ERROR) << "ImageOptimizer rpc failed.";
    }
  }

 private:
  std::unique_ptr<ImageOptimizer::Stub> stub_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  OptimizerClient optimizer_client(
      grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));
  optimizer_client.OptimizeImage();
  return 0;
}
