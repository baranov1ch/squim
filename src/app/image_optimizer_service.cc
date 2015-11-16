#include "app/image_optimizer_service.h"

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using tapoc::ImageRequestPart;
using tapoc::ImageResponsePart;

Status ImageOptimizerService::OptimizeImage(
    ServerContext* context,
    ServerReaderWriter<ImageResponsePart, ImageRequestPart>* stream) {
  ImageRequestPart request_part;
  while (stream->Read(&request_part)) {
    ImageResponsePart response_part;
    response_part.set_message("Hello, " + request_part.name());
    stream->Write(response_part);
  }
  return Status::OK;
}
