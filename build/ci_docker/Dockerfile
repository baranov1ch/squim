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

FROM ubuntu:trusty

RUN apt-get install -y software-properties-common

RUN add-apt-repository ppa:webupd8team/java

RUN apt-get update
RUN echo debconf shared/accepted-oracle-license-v1-1 select true | debconf-set-selections
RUN apt-get install -y \
  oracle-java8-installer \
  pkg-config \
  zip \
  zlib1g-dev \
  unzip \
  curl \
  g++ \
  yasm

RUN curl -L https://github.com/bazelbuild/bazel/releases/download/0.1.1/bazel-0.1.1-installer-linux-x86_64.sh > /tmp/bazel-installer.sh
RUN chmod +x /tmp/bazel-installer.sh

RUN /tmp/bazel-installer.sh

CMD ["bash"]
