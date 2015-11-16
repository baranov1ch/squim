#ifndef APP_IMAGE_OPTIMIZER_SERVICE_H_
#define APP_IMAGE_OPTIMIZER_SERVICE_H_

#include "grpc++/grpc++.h"
#include "proto/image_optimizer.grpc.pb.h"
#include "proto/image_optimizer.pb.h"

class ImageOptimizerService final : public tapoc::ImageOptimizer::Service {
  grpc::Status OptimizeImage(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<tapoc::ImageResponsePart,
                               tapoc::ImageRequestPart>* stream) override;
};

#endif  // APP_IMAGE_OPTIMIZER_SERVICE_H_
