/*
 * Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
