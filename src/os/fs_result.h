#ifndef OS_FS_RESULT_H_
#define OS_FS_RESULT_H_

namespace os {

class FsResult {
 public:
  static FsResult Ok();
  static io::IoResult FromFsResult(const FsResult& result);
  static FsResult Error(OsError os_error,
                        const std::string& op,
                        const std::string& path);

  const char* ToString() const;

 private:
  OsError os_error_;
  std::string op_;
  std::string path_;
};

}  // namespace os

#endif  // OS_FS_RESULT_H_
