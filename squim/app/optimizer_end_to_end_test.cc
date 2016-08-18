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

#include <atomic>
#include <iterator>
#include <fstream>
#include <memory>
#include <thread>

#include "grpc++/grpc++.h"
#include "squim/app/image_optimizer_client.h"
#include "squim/app/optimization.h"
#include "squim/app/request_builder.h"
#include "squim/base/logging.h"
#include "squim/base/memory/make_unique.h"
#include "squim/io/chunk.h"
#include "squim/ioutil/file_util.h"
#include "squim/ioutil/chunk_reader.h"
#include "squim/ioutil/chunk_writer.h"
#include "squim/os/dir_util.h"
#include "squim/os/file.h"
#include "squim/os/file_system.h"

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
using squim::ImageResponsePart_Stats;

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
  std::unique_ptr<ImageOptimizerClient> client_;
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
  auto request_builder = RequestBuilder().SetRecordStats(true).SetQuality(40);
  ImageResponsePart_Stats stats;
  EXPECT_TRUE(client.OptimizeImage(&request_builder, &in, 512, &out, &stats));
  EXPECT_LT(30, stats.psnr());
  auto merged_in = io::Chunk::Merge(jpeg);
  auto merged_out = io::Chunk::Merge(webp);
  EXPECT_LT(merged_out->size(), merged_in->size());
}

TEST_F(OptimizerEndToEndTest, DISABLED_Regressions) {
  ASSERT_TRUE(StartServer());
  ImageOptimizerClient client(
      CreateChannel(kServerAddress, InsecureChannelCredentials()));
  io::ChunkList jpeg;
  ASSERT_TRUE(ioutil::ReadFile("squim/app/testdata/gif/less_then_4096_bytes/"
                               "88916b4bbc2afd4af9abc1dabb9484b0fafcd784a88d023"
                               "4979e1e12427a55f1.gif",
                               &jpeg)
                  .ok());
  io::ChunkList webp;
  ioutil::ChunkListReader in(&jpeg);
  ioutil::ChunkListWriter out(&webp);
  auto request_builder = RequestBuilder().SetRecordStats(true).SetQuality(40);
  ImageResponsePart_Stats stats;
  EXPECT_TRUE(client.OptimizeImage(&request_builder, &in, 512, &out, &stats));
}

TEST_F(OptimizerEndToEndTest, DISABLED_BigIntegrationTest) {
  ASSERT_TRUE(StartServer());

  std::atomic_size_t idx(0u);
  std::vector<std::string> images;

  ASSERT_TRUE(
      os::ReaddirnamesRecursively("squim/app/testdata", 10, &images).ok());

  std::vector<std::thread> clients;
  for (int i = 0; i < 24; ++i) {
    clients.emplace_back(std::thread([&idx, &images]() {
      ImageOptimizerClient client(
          CreateChannel(kServerAddress, InsecureChannelCredentials()));
      size_t thread_idx = idx++;
      for (; thread_idx < images.size(); thread_idx = idx++) {
        auto image = images[thread_idx];
        LOG(INFO) << "Optimizing " << image;
        io::ChunkList src_image;
        ASSERT_TRUE(ioutil::ReadFile(image, &src_image).ok());
        io::ChunkList webp;
        ioutil::ChunkListReader in(&src_image);
        ioutil::ChunkListWriter out(&webp);
        auto request_builder =
            RequestBuilder().SetRecordStats(true).SetQuality(40);
        ImageResponsePart_Stats stats;
        EXPECT_TRUE(
            client.OptimizeImage(&request_builder, &in, 512, &out, &stats))
            << image;
      }
    }));
  }

  for (auto& client_thread : clients)
    client_thread.join();
}
