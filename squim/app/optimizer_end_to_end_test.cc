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

#include "squim/app/image_optimizer_service.h"

#include <iterator>
#include <fstream>
#include <memory>
#include <thread>

#include "grpc++/grpc++.h"
#include "squim/app/image_optimizer_client.h"
#include "squim/app/optimization.h"
#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/io/chunk.h"
#include "squim/ioutil/file_util.h"
#include "squim/ioutil/chunk_reader.h"
#include "squim/ioutil/chunk_writer.h"

#include "gtest/gtest.h"

using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::CreateChannel;
using grpc::InsecureChannelCredentials;
using grpc::InsecureServerCredentials;
using grpc::Server;
using grpc::ServerBuilder;
using squim::ImageOptimizer;
using squim::ImageRequestPart;
using squim::ImageResponsePart;

namespace {
const char kServerAddress[] = "0.0.0.0:50051";
}  // namespace

class OptimizerEndToEndTest : public testing::Test {
 protected:
  void TearDown() override { StopServerIfNecessary(); }

  bool StartServer() {
    service_.reset(
        new ImageOptimizerService(base::make_unique<WebPOptimization>()));
    ServerBuilder builder;
    builder.AddListeningPort(kServerAddress, InsecureServerCredentials());
    builder.RegisterService(service_.get());
    server_ = builder.BuildAndStart();
    server_thread_ = std::thread([this]() { server_->Wait(); });
    return true;
  }

  void StopServerIfNecessary() {
    server_->Shutdown();
    server_thread_.join();
  }

  std::unique_ptr<Server> server_;
  std::unique_ptr<ImageOptimizerService> service_;
  std::thread server_thread_;
};

TEST_F(OptimizerEndToEndTest, SimpleTest) {
  ASSERT_TRUE(StartServer());
  ImageOptimizerClient client(
      CreateChannel(kServerAddress, InsecureChannelCredentials()));
  io::ChunkList jpeg;
  ASSERT_TRUE(ioutil::ReadFile("squim/app/testdata/test.jpg", &jpeg).ok());
  io::ChunkList webp;
  ioutil::ChunkListReader in(&jpeg);
  ioutil::ChunkListWriter out(&webp);
  EXPECT_TRUE(client.OptimizeImage(&in, 512, &out));
  auto merged_in = io::Chunk::Merge(jpeg);
  auto merged_out = io::Chunk::Merge(webp);
  EXPECT_LT(merged_out->size(), merged_in->size());
}
