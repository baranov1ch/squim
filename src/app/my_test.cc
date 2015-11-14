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
