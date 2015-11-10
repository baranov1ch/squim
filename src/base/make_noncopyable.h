#ifndef BASE_MAKE_NONCOPYABLE_H_
#define BASE_MAKE_NONCOPYABLE_H_

#define MAKE_NONCOPYABLE(ClassName)     \
 private:                               \
  ClassName(const ClassName&) = delete; \
  ClassName& operator=(const ClassName&) = delete

#endif  // BASE_MAKE_NONCOPYABLE_H_
