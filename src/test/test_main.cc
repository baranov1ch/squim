#include <stdio.h>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  google::LogToStderr();
  google::InstallFailureSignalHandler();
  testing::InitGoogleTest(&argc, argv);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
