#include <memory>

#include "glog/logging.h"
#include "webp/encode.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "proto/optimizer.grpc.pb.h"
#include "proto/optimizer.pb.h"

DEFINE_bool(do_nothing, true, "If true, does nothing");

class GreeterServiceImpl final : public optimizer::Greeter::Service {
  grpc::Status SayHello(grpc::ServerContext* context, const optimizer::HelloRequest* request,
                        optimizer::HelloReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return grpc::Status::OK;
  }
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Hello, World! " << FLAGS_do_nothing;
  std::string server_address("0.0.0.0:50051");
  GreeterServiceImpl service;
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
