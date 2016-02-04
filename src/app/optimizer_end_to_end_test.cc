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

#include "app/image_optimizer_client.h"
#include "app/optimization.h"
#include "base/logging.h"
#include "base/memory/make_unique.h"
#include "grpc++/grpc++.h"
#include "io/chunk.h"
#include "ioutil/chunk_reader.h"
#include "ioutil/chunk_writer.h"
#include "ioutil/file_util.h"

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
}

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
  io::ChunkList original_image;
  io::ChunkList webp_image;
  auto result =
      ioutil::ReadFileChunks("app/testdata/test.jpg", &original_image);
  ASSERT_TRUE(result.ok());
  ioutil::ChunkListReader in(original_image);
  ioutil::ChunkListWriter out(&webp_image);
  ImageOptimizerClient client(
      CreateChannel(kServerAddress, InsecureChannelCredentials()));
  EXPECT_TRUE(client.OptimizeImage(&in, 512, &out));
  auto merged_input = io::Chunk::Merge(original_image);
  auto merged_output = io::Chunk::Merge(webp_image);
  EXPECT_GT(merged_input->size(), merged_output->size());
}
