# Copyright 2015 Alexey Baranov <me@kotiki.cc>. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])

load("/tools/build_rules/gen_config_header", "gen_config_header")
load("/tools/build_rules/gen_headers", "gen_headers")

gen_config_header(
  name = "config_h",
  src = "upstream/src/config.h.cmake.in",
  out = "upstream/src/config.h",
  replacements = {
    "#cmakedefine DISABLE_RTTI": "#define DISABLE_RTTI 1",
    "#cmakedefine GOOGLE_NAMESPACE ${GOOGLE_NAMESPACE}": "#define GOOGLE_NAMESPACE google",
    "#cmakedefine HAVE_DLADDR": "#define HAVE_DLADDR 1",
    "#cmakedefine HAVE_SNPRINTF": "#define HAVE_SNPRINTF 1",
    "#cmakedefine HAVE_DLFCN_H": "#define HAVE_DLFCN_H 1",
    "#cmakedefine HAVE_EXECINFO_H": "#define HAVE_EXECINFO_H 1",
    "#cmakedefine HAVE_FCNTL": "#define HAVE_FCNTL 1",
    "#cmakedefine HAVE_GLOB_H": "#define HAVE_GLOB_H 1",
    "#cmakedefine HAVE_INTTYPES_H ${HAVE_INTTYPES_H}": "/* #undef HAVE_INTTYPES_H */",
    "#cmakedefine HAVE_LIBPTHREAD": "#define HAVE_LIBPTHREAD 1",
    "#cmakedefine HAVE_LIBUNWIND_H": "/* #undef HAVE_LIBUNWIND_H */",
    "#cmakedefine HAVE_LIB_GFLAGS": "#define HAVE_LIB_GFLAGS 1",
    "#cmakedefine HAVE_LIB_GMOCK": "#define HAVE_LIB_GMOCK 1",
    "#cmakedefine HAVE_LIB_GTEST": "#define HAVE_LIB_GTEST 1",
    "#cmakedefine HAVE_LIB_UNWIND": "/* #undef HAVE_LIB_GFLAGS */",
    "#cmakedefine HAVE_MEMORY_H": "#define HAVE_MEMORY_H 1",
    "#cmakedefine NO_THREADS": "/* #undef NO_THREADS */",
    "#cmakedefine HAVE_NAMESPACES": "#define HAVE_NAMESPACES 1",
    "#cmakedefine HAVE_PREAD": "#define HAVE_PREAD 1",
    "#cmakedefine HAVE_PTHREAD": "#define HAVE_PTHREAD 1",
    "#cmakedefine HAVE_PWD_H": "#define HAVE_PWD_H 1",
    "#cmakedefine HAVE_PWRITE": "#define HAVE_PWRITE 1",
    "#cmakedefine HAVE_RWLOCK": "/* #undef HAVE_RWLOCK */",
    "#cmakedefine HAVE_SIGACTION": "#define HAVE_SIGACTION 1",
    "#cmakedefine HAVE_SIGALTSTACK": "#define HAVE_SIGALTSTACK 1",
    "#cmakedefine HAVE_STDINT_H ${HAVE_STDINT_H}": "#define HAVE_STDINT_H 1",
    "#cmakedefine HAVE_STDLIB_H": "#define HAVE_STDLIB_H 1",
    "#cmakedefine HAVE_STRINGS_H": "/* #undef HAVE_STRINGS_H */",
    "#cmakedefine HAVE_STRING_H": "#define HAVE_STRING_H 1",
    "#cmakedefine HAVE_SYSCALL_H": "#define HAVE_SYSCALL_H 1",
    "#cmakedefine HAVE_SYSLOG_H": "#define HAVE_SYSLOG_H 1",
    "#cmakedefine HAVE_SYS_STAT_H": "#define HAVE_SYS_STAT_H 1",
    "#cmakedefine HAVE_SYS_SYSCALL_H": "#define HAVE_SYS_SYSCALL_H 1",
    "#cmakedefine HAVE_SYS_TIME_H": "#define HAVE_SYS_TIME_H 1",
    "#cmakedefine HAVE_SYS_TYPES_H ${HAVE_SYS_TYPES_H}": "#define HAVE_SYS_TYPES_H 1",
    "#cmakedefine HAVE_SYS_UCONTEXT_H": "#define HAVE_SYS_UCONTEXT_H 1",
    "#cmakedefine HAVE_SYS_UTSNAME_H": "#define HAVE_SYS_UTSNAME_H 1",
    "#cmakedefine HAVE_UCONTEXT_H": "#define HAVE_UCONTEXT_H 1",
    "#cmakedefine HAVE_UNISTD_H ${HAVE_UNISTD_H}": "#define HAVE_UNISTD_H 1",
    "#cmakedefine HAVE_UNWIND_H ${HAVE_UNWIND_H}": "#define HAVE_UNWIND_H 1",
    "#cmakedefine HAVE_USING_OPERATOR": "#define HAVE_USING_OPERATOR 1",
    "#cmakedefine HAVE___ATTRIBUTE__": "#define HAVE___ATTRIBUTE__ 1",
    "#cmakedefine HAVE___BUILTIN_EXPECT ${HAVE___BUILTIN_EXPECT}": "#define HAVE___BUILTIN_EXPECT 1",
    "#cmakedefine HAVE___SYNC_VAL_COMPARE_AND_SWAP": "#define HAVE___SYNC_VAL_COMPARE_AND_SWAP 1",
    "#cmakedefine LT_OBJDIR": "/* #undef LT_OBJDIR */",
    "#cmakedefine PACKAGE": "#define PACKAGE \"glog\"",
    "#cmakedefine PACKAGE_BUGREPORT": "#define PACKAGE_BUGREPORT \"https://github.com/google/glog\"",
    "#cmakedefine PACKAGE_NAME": "#define PACKAGE_NAME \"glog\"",
    "#cmakedefine PACKAGE_STRING": "#define PACKAGE_STRING \"glog 0.3.4\"",
    "#cmakedefine PACKAGE_TARNAME": "#define PACKAGE_TARNAME \"glog-0.3.4\"",
    "#cmakedefine PACKAGE_URL": "#define PACKAGE_URL \"https://github.com/google/glog\"",
    "#cmakedefine PACKAGE_VERSION": "#define PACKAGE_VERSION \"0.3.4\"",
    "#cmakedefine PC_FROM_UCONTEXT": "#define PC_FROM_UCONTEXT uc_mcontext.gregs[REG_RIP]",
    "#cmakedefine PTHREAD_CREATE_JOINABLE": "/* #undef PTHREAD_CREATE_JOINABLE */",
    "#cmakedefine SIZEOF_VOID_P ${SIZEOF_VOID_P}": "#define SIZEOF_VOID_P 8",
    "#cmakedefine STDC_HEADERS": "/* #undef STDC_HEADERS */",
    "#cmakedefine STL_NAMESPACE ${STL_NAMESPACE}": "#define STL_NAMESPACE std",
    "#cmakedefine TEST_SRC_DIR ${TEST_SRC_DIR}": "#define TEST_SRC_DIR \".\"",
    "#cmakedefine VERSION": "#define VERSION \"0.3.4\"",
    "#cmakedefine _END_GOOGLE_NAMESPACE_ ${_END_GOOGLE_NAMESPACE_}": "#define _END_GOOGLE_NAMESPACE_ }",
    "#cmakedefine _START_GOOGLE_NAMESPACE_ ${_START_GOOGLE_NAMESPACE_}": "#define _START_GOOGLE_NAMESPACE_ namespace google {",
  },
)

gen_headers(
  name = "configure",
  files = [
    "upstream/src/glog/logging",
    "upstream/src/glog/raw_logging",
    "upstream/src/glog/stl_logging",
    "upstream/src/glog/vlog_is_on",
  ],
  replacements = {
    "@ac_cv_have_unistd_h@": "1",
    "@ac_cv_have_stdint_h@": "1",
    "@ac_cv_have_systypes_h@": "1",
    "@ac_cv_have_inttypes_h@": "0",
    "@ac_cv_have_libgflags@": "0",
    "@ac_cv_have_uint16_t@": "1",
    "@ac_cv_have_u_int16_t@": "0",
    "@ac_cv_have___uint16@": "0",
    "@ac_cv_have___builtin_expect@": "1",
    "@ac_cv_cxx_using_operator@": "1",
    "@ac_cv___attribute___noinline@": "__attribute__((noinline))",
    "@ac_cv___attribute___noreturn@": "__attribute__((noreturn))",
    "@ac_cv___attribute___printf_4_5@": "__attribute__((__format__ (__printf__, 4, 5)))",
    "@ac_google_start_namespace@": "namespace google {",
    "@ac_google_end_namespace@": "}",
    "@ac_google_namespace@": "google",
  },
)

cc_library(
  name = "glog",
  linkstatic = 1,
  defines = [
    "GOOGLE_GLOG_DLL_DECL=",
  ],
  hdrs = [
    ":upstream/src/config.h",
    ":upstream/src/glog/logging.h",
    ":upstream/src/glog/raw_logging.h",
    ":upstream/src/glog/stl_logging.h",
    ":upstream/src/glog/vlog_is_on.h",
    "upstream/src/base/commandlineflags.h",
    "upstream/src/base/googleinit.h",
    "upstream/src/base/mutex.h",
    "upstream/src/demangle.h",
    ":upstream/src/glog/log_severity.h",
    "upstream/src/mock-log.h",
    "upstream/src/stacktrace.h",
    "upstream/src/symbolize.h",
    "upstream/src/utilities.h",
  ],
  srcs = [
    "upstream/src/demangle.cc",
    "upstream/src/logging.cc",
    "upstream/src/raw_logging.cc",
    "upstream/src/signalhandler.cc",
    "upstream/src/stacktrace_powerpc-inl.h",
    "upstream/src/stacktrace_x86-inl.h",
    "upstream/src/stacktrace_x86_64-inl.h",
    "upstream/src/symbolize.cc",
    "upstream/src/utilities.cc",
    "upstream/src/vlog_is_on.cc",
  ],
  includes = [
    "upstream/src",
    "../gflags/upstream/include"
  ],
  copts = [
    "-Wno-sign-compare",
  ],
  deps = [
    "//google/gflags:gflags",
  ],
  visibility = ["//visibility:public"]
)
