#!/usr/bin/env bash

# This must be the same C++ compiler used to build the LLVM source.
if [[ -z "$CLANG" ]]; then
  if [[ -z "$(which clang)" ]]; then
    echo 'You need to have clang installed to build squim.'
    echo 'Note: Some vendors install clang with a versioned name'
    echo '(like /usr/bin/clang-3.5). You can set the CLANG environment'
    echo 'variable to specify the full path to yours.'
    exit 1
  fi
  CLANG="$(realpath -s $(which clang))"
fi

echo "Using clang found at ${CLANG}" >&2

# The C++ compiler looks at some fixed but version-specific paths in the local
# filesystem for headers. We can't predict where these will be on users'
# machines because (among other reasons) they sometimes include the version
# number of the compiler being used. We can interrogate Clang (and gcc, which
# thankfully has a similar enough output format) for these paths.

# We use realpath -s as well as readlink -e to allow compilers to employ various
# methods for path canonicalization (and because Bazel may not always allow paths
# with relative arcs in its whitelist).

BUILTIN_INCLUDES=$(${CLANG} -E -x c++ - -v 2>&1 < /dev/null \
  | sed -n '/search starts here\:/,/End of search list/p' \
  | sed '/#include.*/d
/End of search list./d' \
  | while read -r INCLUDE_PATH ; do
  printf "%s" "  cxx_builtin_include_directory: \"$(realpath -s ${INCLUDE_PATH})\"__EOL__"
if [[ $(uname) != 'Darwin' ]]; then
  printf "%s" "  cxx_builtin_include_directory: \"$(readlink -e ${INCLUDE_PATH})\"__EOL__"
fi
done)

sed "s|ADD_CXX_COMPILER|${CLANG}|g
s|ADD_CXX_BUILTIN_INCLUDE_DIRECTORIES|${BUILTIN_INCLUDES}|g" \
    tools/cpp/CROSSTOOL.in | \
sed 's|__EOL__|\
|g' > tools/cpp/CROSSTOOL
