#include "io/buf_reader.h"

#include <memory>

#include "base/make_unique.h"
#include "io/buffered_source.h"
#include "io/chunk.h"

#include "gtest/gtest.h"

namespace io {

namespace {
std::string StringFromBytes(uint8_t* bytes, uint64_t len) {
  auto* chars = reinterpret_cast<const char*>(bytes);
  return std::string(chars, len);
}
}

class BufReaderTest : public ::testing::Test {
 public:
  void SetUp() override {
    auto source = base::make_unique<BufferedSource>();
    testee_ = base::make_unique<BufReader>(std::move(source));
  }

 protected:
  std::unique_ptr<BufReader> testee_;
};

TEST_F(BufReaderTest, ReadPending) {
  uint8_t* out;
  EXPECT_TRUE(testee_->ReadSome(&out).pending());
  EXPECT_TRUE(testee_->ReadAtMostN(&out, 100).pending());
  EXPECT_TRUE(testee_->ReadN(&out, 100).pending());

  std::unique_ptr<uint8_t[]> buf(new uint8_t[20]);
  EXPECT_TRUE(testee_->ReadNInto(out, 20).pending());
  EXPECT_TRUE(testee_->PeekNInto(out, 20).pending());
}

TEST_F(BufReaderTest, ReadEOF) {
  testee_->source()->SendEof();
  uint8_t* out;
  EXPECT_TRUE(testee_->ReadSome(&out).eof());
  EXPECT_TRUE(testee_->ReadAtMostN(&out, 100).eof());
  EXPECT_TRUE(testee_->ReadN(&out, 100).eof());

  std::unique_ptr<uint8_t[]> buf(new uint8_t[20]);
  EXPECT_TRUE(testee_->ReadNInto(out, 20).eof());
  EXPECT_TRUE(testee_->PeekNInto(out, 20).eof());
}

TEST_F(BufReaderTest, Reading) {
  testee_->source()->AddChunk(base::make_unique<StringChunk>("test1"));
  uint8_t* out;
  EXPECT_EQ(5, testee_->ReadSome(&out).nread());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
  EXPECT_EQ(5, testee_->UnreadN(5));

  EXPECT_EQ(5, testee_->ReadAtMostN(&out, 10).nread());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
  EXPECT_EQ(5, testee_->UnreadN(5));

  EXPECT_EQ(5, testee_->ReadN(&out, 5).nread());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
  EXPECT_EQ(5, testee_->UnreadN(5));

  std::unique_ptr<uint8_t[]> buf(new uint8_t[10]);
  EXPECT_EQ(5, testee_->PeekNInto(out, 5).nread());
  EXPECT_EQ("test1", StringFromBytes(out, 5));

  EXPECT_EQ(5, testee_->ReadNInto(out, 5).nread());
  EXPECT_EQ("test1", StringFromBytes(out, 5));
}

}  // namespace io
