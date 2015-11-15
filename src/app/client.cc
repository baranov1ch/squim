#include <memory>

#include "glog/logging.h"
#include "gflags/gflags.h"
#include "grpc++/grpc++.h"
#include "proto/optimizer.grpc.pb.h"
#include "proto/optimizer.pb.h"

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(optimizer::Greeter::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    optimizer::HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    optimizer::HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;

    // The actual RPC.
    grpc::Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<optimizer::Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Hello, World from client ";
  GreeterClient greeter(
      grpc::CreateChannel("localhost:50051", grpc::InsecureCredentials()));
  std::string user("world");
  std::string reply = greeter.SayHello(user);
  LOG(INFO) << "Greeter received: " << reply;
}
