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

namespace {

bool ReadFile(const std::string& path, std::vector<uint8_t>* contents) {
  DCHECK(contents);
  contents->clear();
  std::fstream file(path, std::ios::in | std::ios::binary);
  if (!file.good())
    return false;

  std::vector<char> tmp((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
  contents->reserve(tmp.size());
  for (const auto& c : tmp)
    contents->push_back(static_cast<uint8_t>(c));
  return true;
}

bool WriteFile(const std::string& path, const std::vector<uint8_t>& data) {
  std::vector<char> hack;
  hack.reserve(data.size());
  for (auto c : data)
    hack.push_back(static_cast<char>(c));
  std::ofstream output_file(path);
  std::ostreambuf_iterator<char> out(output_file);
  std::copy(hack.begin(), hack.end(), out);
  return true;
}

DEFINE_string(in, "test.png", "input image file");
DEFINE_string(out, "test.webp", "output file");

}  // namespace

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  std::vector<uint8_t> in;
  std::vector<uint8_t> out;
  if (!ReadFile(FLAGS_in, &in)) {
    LOG(ERROR) << "Cannot read input file " << FLAGS_in;
    return 1;
  }

  LOG(INFO) << "Sending picture, size=" << in.size();

  ImageOptimizerClient client(
      grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));
  if (!client.OptimizeImage(in, 1024, &out)) {
    LOG(ERROR) << "Optimization failed";
    return 1;
  }

  LOG(INFO) << "Optimized picture received, size=" << out.size();

  WriteFile(FLAGS_out, out);
  return 0;
}
