#ifndef OS_FILE_MODE_H_
#define OS_FILE_MODE_H_

namespace os {

class FileMode {
 public:
  enum {
    kDir = 1 << 31,
    kAppend = 1 << 30,
    kExclusive = 1 << 29,
    kTemporary = 1 << 28,
    kSymlink = 1 << 27,
    kDevice = 1 << 26,
    kNamedPipe = 1 << 25,
    kSocket = 1 << 24,
    kSetuid = 1 << 23,
    kSetgid = 1 << 22,
    kCharDevice = 1 << 21,
    kSticky = 1 << 20,
    kType = kDir | kSymlink | kNamedPipe | kSocket | kDevice,
    kPerm = 0777,
  };
  explicit FileMode(uint32_t os_perm) os_perm_(os_perm) {}

  bool is_dir() const { return os_perm_ & kDir != 0; }
  bool is_regular() const { os_perm_& kType == 0; }
  FileMode perm() const { return FileMode(os_perm_ & kPerm); }

 private:
  uint32_t os_perm_;
};

}  // namespace os

#endif  // OS_FILE_MODE_H_
