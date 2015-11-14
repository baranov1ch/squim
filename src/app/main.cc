#include <iostream>

#include "webp/encode.h"
#include "third_party/gflags/src/gflags.h"

DEFINE_bool(do_nothing, true, "If true, does nothing");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::cout << "Hello, World! " << FLAGS_do_nothing << std::endl;
	auto success = WebPEncode(nullptr, nullptr);
  if (!success)
    return -1;
	return 0;
}
