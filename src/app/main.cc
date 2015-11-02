#include <iostream>

#include "webp/encode.h"

int main() {
  std::cout << "Hello, World!" << std::endl;
	auto success = WebPEncode(nullptr, nullptr);
  if (!success)
    return -1;
	return 0;
}
