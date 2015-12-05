#ifndef OS_FILE_SYSTEM_H_
#define OS_FILE_SYSTEM_H_

#include <memory>
#include <string>

#include "os/fs_result.h"
#include "os/file_mode.h"

namespace os {

class File;
class FileInfo;

class FileSystem {
 public:
  virtual FsResult Open(const std::string& path,
                        std::unique_ptr<File>* file) = 0;
  virtual FsResult Create(const std::string& path,
                          std::unique_ptr<File>* file) = 0;
  virtual FsResult OpenFile(const std::string& path,
                            int flags,
                            FileMode permission_bits,
                            std::unique_ptr<File>* file) = 0;
  virtual FsResult MkDir(const std::string& path, FileMode permission_bits) = 0;
  virtual FsResult MkDirP(const std::string& path,
                          FileMode permission_bits) = 0;
  virtual FsResult Remove(const std::string& path) = 0;
  virtual FsResult RemoveAll(const std::string& path) = 0;
  virtual FsResult Rename(const std::string& old_path,
                          const std::string& new_path) = 0;
  virtual std::string TempDir() = 0;
  virtual FsResult Lstat(const std::string& path, FileInfo* file_info) = 0;
  virtual FsResult Stat(const std::string& path, FileInfo* file_info) = 0;
  virtual FileReader* Stdout() const = 0;
  virtual FileReader* Stderr() const = 0;
  virtual FileWriter* Stdin() const = 0;

  virtual ~FileSystem() {}
};

}  // namespace os

#endif  // OS_FILE_SYSTEM_H_
