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

#include "app/image_optimizer_service.h"

#include <memory>
#include <thread>

#include "glog/logging.h"
#include "grpc++/grpc++.h"

#include "gtest/gtest.h"

using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::CreateChannel;
using grpc::InsecureCredentials;
using grpc::InsecureServerCredentials;
using grpc::Server;
using grpc::ServerBuilder;
using tapoc::ImageOptimizer;
using tapoc::ImageRequestPart;
using tapoc::ImageResponsePart;

namespace {
const char kServerAddress[] = "0.0.0.0:50051";
}

class OptimizerEndToEndTest : public testing::Test {
 protected:
  void TearDown() override { StopServerIfNecessary(); }

  bool StartServerAndClient() {
    service_.reset(new ImageOptimizerService);
    ServerBuilder builder;
    builder.AddListeningPort(kServerAddress, InsecureServerCredentials());
    builder.RegisterService(service_.get());
    server_ = builder.BuildAndStart();
    server_thread_ = std::thread([this]() { server_->Wait(); });

    auto channel = CreateChannel(kServerAddress, InsecureCredentials());
    stub_ = ImageOptimizer::NewStub(channel);
    return true;
  }

  void StopServerIfNecessary() {
    server_->Shutdown();
    server_thread_.join();
  }

  std::shared_ptr<ClientReaderWriter<ImageRequestPart, ImageResponsePart>>
  CreateStream() {
    return stub_->OptimizeImage(&context_);
  }

  std::unique_ptr<Server> server_;
  std::unique_ptr<ImageOptimizerService> service_;
  std::thread server_thread_;
  std::unique_ptr<ImageOptimizer::Stub> stub_;
  ClientContext context_;
};

TEST_F(OptimizerEndToEndTest, SimpleTest) {
  EXPECT_TRUE(StartServerAndClient());
  auto stream = CreateStream();

  std::thread writer([stream]() {
    for (int i = 0; i < 5; ++i) {
      ImageRequestPart part;
      part.set_name("world");
      stream->Write(part);
    }
    stream->WritesDone();
  });

  ImageResponsePart response_part;
  while (stream->Read(&response_part)) {
    EXPECT_EQ("Hello, world", response_part.message());
  }
  writer.join();
}
