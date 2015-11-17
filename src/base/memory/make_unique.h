#ifndef BASE_MEMORY_MAKE_UNIQUE_H_
#define BASE_MEMORY_MAKE_UNIQUE_H_

#include <memory>

namespace base {

template <class T, class... Args>
inline std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace base

#endif  // BASE_MEMORY_MAKE_UNIQUE_H_
