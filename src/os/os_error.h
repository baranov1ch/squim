#ifndef OS_OS_ERROR_H_
#define OS_OS_ERROR_H_

namespace os {

class OsError {
 public:
  static OsError Ok();
  static OsError Error(int error_code);

  const char* ToString() const;

 private:
  int os_error_code_;
  std::string message_;
};

}  // namespace os

#endif  // OS_OS_ERROR_H_
