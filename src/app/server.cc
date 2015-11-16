#include <memory>

#include "glog/logging.h"
#include "webp/encode.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "proto/image_optimizer.grpc.pb.h"
#include "proto/image_optimizer.pb.h"

DEFINE_bool(do_nothing, true, "If true, does nothing");

class ImageOptimizerServiceImpl final : public tapoc::ImageOptimizer::Service {
  grpc::Status OptimizeImage(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<tapoc::ImageResponsePart,
                               tapoc::ImageRequestPart>* stream) override {
    tapoc::ImageRequestPart request_part;
    while (stream->Read(&request_part)) {
      tapoc::ImageResponsePart response_part;
      response_part.set_message("Hello, " + request_part.name());
      stream->Write(response_part);
    }
    return grpc::Status::OK;
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  LOG(INFO) << "Hello, World! " << FLAGS_do_nothing;
  std::string server_address("0.0.0.0:50051");
  ImageOptimizerServiceImpl service;
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
