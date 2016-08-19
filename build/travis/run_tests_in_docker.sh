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

set -ex

cd `dirname $0`/../..
git_root=`pwd`
cd -

DOCKER_IMAGE_NAME=squim_docker_`sha1sum ${git_root}/build/travis/Dockerfile | cut -f1 -d\ `

echo $DOCKER_IMAGE_NAME

docker build -t $DOCKER_IMAGE_NAME ${git_root}/build/travis

CONTAINER_NAME="travis_run_tests_$(uuidgen)"

echo $CONTAINER_NAME

opts="--verbose_failures  -c dbg"
tests=`cat ${git_root}/build/testsuites | tr "\n" " "`

COMMAND="bazel test ${tests} ${opts}"

echo $COMMAND

docker run \
  -e "COMMAND=$COMMAND" \
  -v "$git_root:/var/local/ci/squim" \
  -w /var/local/ci/squim/ \
  --name=$CONTAINER_NAME \
  $DOCKER_IMAGE_NAME \
  bash -l ./build/travis/setup_bazel_and_run_tests.sh || DOCKER_FAILED="true"

docker rm -f $CONTAINER_NAME || true

if [ "$DOCKER_FAILED" != "" ]
then
  exit 1
fi
