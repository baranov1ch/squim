#include <iostream>
#include <sys/utsname.h>
#include <unwind.h>
#include <syscall.h>
#include <sys/syscall.h>

#include "glog/logging.h"
#include "webp/encode.h"
#include "gflags/gflags.h"

DEFINE_bool(do_nothing, true, "If true, does nothing");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "Hello, World! " << FLAGS_do_nothing;
	auto success = WebPEncode(nullptr, nullptr);
  if (!success)
    return -1;
	return 0;
}
