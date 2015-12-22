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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::Return;

class TestClass {
 public:
  virtual ~TestClass() {}

  int DoStuff() { return DoStuffImpl(); }

  virtual int DoStuffImpl() = 0;
};

class MockTestClass : public TestClass {
 public:
  MOCK_METHOD0(DoStuffImpl, int());
};

TEST(FactorialTest, Negative) {
  MockTestClass mock;
  EXPECT_CALL(mock, DoStuffImpl()).WillOnce(Return(5));
  EXPECT_EQ(5, mock.DoStuff());
}
