#include <memory>
#include <thread>

#include "glog/logging.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "proto/image_optimizer.grpc.pb.h"
#include "proto/image_optimizer.pb.h"

tapoc::ImageRequestPart MakeImageRequestPart(const std::string& name) {
  tapoc::ImageRequestPart p;
  p.set_name(name);
  return p;
}

class OptimizerClient {
 public:
  OptimizerClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(tapoc::ImageOptimizer::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  void OptimizeImage() {
    grpc::ClientContext context;

    std::shared_ptr<grpc::ClientReaderWriter<tapoc::ImageRequestPart,
                                             tapoc::ImageResponsePart>>
        stream(stub_->OptimizeImage(&context));

    std::thread writer([stream]() {
      std::vector<tapoc::ImageRequestPart> parts{
          MakeImageRequestPart("First message"),
          MakeImageRequestPart("Second message"),
          MakeImageRequestPart("Third message"),
          MakeImageRequestPart("Fourth message")};
      for (const auto& part : parts) {
        LOG(INFO) << "Sending message " << part.name();
        stream->Write(part);
      }
      stream->WritesDone();
    });

    tapoc::ImageResponsePart response_part;
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
  std::unique_ptr<tapoc::ImageOptimizer::Stub> stub_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Hello, World from client ";
  OptimizerClient optimizer_client(
      grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));
  optimizer_client.OptimizeImage();
  return 0;
}
