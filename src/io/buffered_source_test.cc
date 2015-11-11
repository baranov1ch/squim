#include "gtest/gtest.h"

#include "base/make_unique.h"
#include "io/buffered_source.h"
#include "io/chunk.h"

namespace io {

namespace {

std::string StringFromBytes(uint8_t* bytes, uint64_t len) {
  auto* chars = reinterpret_cast<const char*>(bytes);
  return std::string(chars, len);
}

struct ReadCase {
  const char* data;
  bool have_some;
};

struct PartialReadCase {
  const char* data;
  int n;
  bool have_some;
};
}

class BufferedSourceTest : public ::testing::Test {
 protected:
  template <size_t N>
  void RunFullCases(const ReadCase(&cases)[N]) {
    uint64_t expected_size = 0;
    for (auto cs : cases) {
      expected_size += strlen(cs.data);
      testee_.AddChunk(base::make_unique<StringChunk>(cs.data));
    }
    EXPECT_EQ(expected_size, testee_.size());

    uint64_t offset = 0;
    for (const auto& cs : cases) {
      ASSERT_TRUE(testee_.HaveSome());
      uint8_t* out;
      auto nread = testee_.ReadSome(&out);
      auto expected_len = strlen(cs.data);
      offset += expected_len;
      EXPECT_EQ(expected_len, static_cast<size_t>(nread));
      EXPECT_EQ(offset, testee_.offset());
      EXPECT_EQ(cs.data, StringFromBytes(out, nread));
      EXPECT_EQ(cs.have_some, testee_.HaveSome());
    }
  }

  template <size_t M, size_t N>
  void ReadPartial(const char*(&chunks)[M], const PartialReadCase(&reads)[N]) {
    uint64_t expected_size = 0;
    for (auto chunk : chunks) {
      expected_size += strlen(chunk);
      testee_.AddChunk(base::make_unique<StringChunk>(chunk));
    }
    EXPECT_EQ(expected_size, testee_.size());

    uint64_t offset = 0;
    for (const auto& r : reads) {
      ASSERT_TRUE(testee_.HaveSome());
      uint8_t* out;
      uint64_t nread;
      if (r.n >= 0) {
        nread = testee_.ReadAtMostN(&out, r.n);
      } else {
        nread = testee_.ReadSome(&out);
      }
      auto expected_len = strlen(r.data);
      offset += expected_len;
      EXPECT_EQ(expected_len, static_cast<size_t>(nread));
      EXPECT_EQ(offset, testee_.offset());
      EXPECT_EQ(r.data, StringFromBytes(out, nread));
      EXPECT_EQ(r.have_some, testee_.HaveSome());
    }
  }

  BufferedSource testee_;
};

TEST_F(BufferedSourceTest, ShouldNotHaveSomeWhenEmpty) {
  EXPECT_FALSE(testee_.HaveSome());
}

TEST_F(BufferedSourceTest, ShouldNotAddEmptyChunks) {
  testee_.AddChunk(base::make_unique<StringChunk>(std::string()));
  EXPECT_FALSE(testee_.HaveSome());
}

TEST_F(BufferedSourceTest, ShouldHaveSomeWhenItShould) {
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  EXPECT_TRUE(testee_.HaveSome());
}

TEST_F(BufferedSourceTest, ShouldReadFullChunk) {
  ReadCase kCases[] = {
      {"test", false},
  };

  RunFullCases(kCases);
}

TEST_F(BufferedSourceTest, SequentialReadByChunk) {
  ReadCase kCases[] = {
      {"test", true}, {"me", true}, {"again", false},
  };

  RunFullCases(kCases);
}

TEST_F(BufferedSourceTest, SequentialReadParts) {
  const char* kChunks[] = {
      "test", "me", "again", "andagain",
  };

  PartialReadCase kCases[] = {
      {"tes", 3, true},  {"t", 4, true}, {"me", 4, true},  {"ag", 2, true},
      {"ain", -1, true}, {"a", 1, true}, {"nda", 3, true}, {"gain", 4, false},
  };

  ReadPartial(kChunks, kCases);
}

TEST_F(BufferedSourceTest, Refill) {
  EXPECT_FALSE(testee_.HaveSome());
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  EXPECT_TRUE(testee_.HaveSome());
  uint8_t* out;
  EXPECT_EQ(4, testee_.ReadSome(&out));
  EXPECT_FALSE(testee_.HaveSome());

  // Now add again - we should see the new chunk
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  EXPECT_TRUE(testee_.HaveSome());
  EXPECT_EQ(4, testee_.ReadSome(&out));
}

TEST_F(BufferedSourceTest, HaveN) {
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  EXPECT_TRUE(testee_.HaveN(3));
  EXPECT_TRUE(testee_.HaveN(6));
  EXPECT_TRUE(testee_.HaveN(11));
  EXPECT_FALSE(testee_.HaveN(13));

  // Move offset to the middle of the first buffer and re-check.
  uint8_t* out;
  EXPECT_EQ(3, testee_.ReadAtMostN(&out, 3));
  EXPECT_TRUE(testee_.HaveN(2));
  EXPECT_TRUE(testee_.HaveN(3));
  EXPECT_TRUE(testee_.HaveN(6));
  EXPECT_FALSE(testee_.HaveN(11));
}

TEST_F(BufferedSourceTest, ShouldReturnEOFOnlyWhenEmptyAndReceived) {
  EXPECT_FALSE(testee_.EofReached());
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  EXPECT_FALSE(testee_.EofReached());

  uint8_t* out;
  EXPECT_EQ(4, testee_.ReadSome(&out));
  // After read the buffer is depleted, but still no Eof.
  EXPECT_FALSE(testee_.HaveSome());
  EXPECT_FALSE(testee_.EofReached());

  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  testee_.SendEof();

  // No EOF - we still have data to read.
  EXPECT_FALSE(testee_.EofReached());

  EXPECT_EQ(4, testee_.ReadSome(&out));

  // Only now when all data read, we should signal EOF.
  EXPECT_FALSE(testee_.HaveSome());
  EXPECT_TRUE(testee_.EofReached());

  // No data should be accepted after EOF.
  testee_.AddChunk(base::make_unique<StringChunk>("test"));
  EXPECT_FALSE(testee_.HaveSome());
  EXPECT_EQ(8, testee_.size());
}

TEST_F(BufferedSourceTest, Unreading) {
  testee_.AddChunk(base::make_unique<StringChunk>("test1"));
  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  uint8_t* out;
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_FALSE(testee_.HaveSome());
  EXPECT_EQ(3, testee_.UnreadN(3));
  EXPECT_EQ(3, testee_.ReadSome(&out));
  EXPECT_EQ("st2", StringFromBytes(out, 3));
  EXPECT_EQ(1, testee_.UnreadN(1));
  EXPECT_EQ(6, testee_.UnreadN(6));
  EXPECT_EQ(2, testee_.ReadSome(&out));
  EXPECT_EQ("t1", StringFromBytes(out, 2));
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ("test2", StringFromBytes(out, 5));
}

TEST_F(BufferedSourceTest, ReadWithMerge) {
  testee_.AddChunk(base::make_unique<StringChunk>("test1"));
  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  testee_.AddChunk(base::make_unique<StringChunk>("test3"));
  ASSERT_TRUE(testee_.HaveN(7));
  uint8_t* out;
  EXPECT_EQ(7, testee_.ReadN(&out, 7));
  EXPECT_EQ("test1te", StringFromBytes(out, 7));
  ASSERT_FALSE(testee_.HaveN(9));
  ASSERT_TRUE(testee_.HaveN(8));
  EXPECT_EQ(2, testee_.ReadN(&out, 2));
  EXPECT_EQ("st", StringFromBytes(out, 2));
  EXPECT_EQ(3, testee_.ReadN(&out, 3));
  EXPECT_EQ("2te", StringFromBytes(out, 3));
  ASSERT_FALSE(testee_.HaveN(4));
  ASSERT_TRUE(testee_.HaveN(3));
  EXPECT_EQ(12, testee_.UnreadN(12));

  // After all that, all chunks should be merged into one.
  EXPECT_EQ(15, testee_.ReadSome(&out));
  EXPECT_EQ("test1test2test3", StringFromBytes(out, 15));
}

TEST_F(BufferedSourceTest, Freeing) {
  testee_.AddChunk(base::make_unique<StringChunk>("test1"));
  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  EXPECT_EQ(0, testee_.FreeAsMuchAsPossible());

  uint8_t* out;
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(5, testee_.offset());
  EXPECT_EQ(10, testee_.size());
  EXPECT_EQ(5, testee_.FreeAsMuchAsPossible());
  EXPECT_EQ(0, testee_.offset());
  EXPECT_EQ(5, testee_.size());

  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  EXPECT_EQ(3, testee_.ReadAtMostN(&out, 3));
  EXPECT_EQ(0, testee_.FreeAsMuchAsPossible());

  EXPECT_EQ(2, testee_.ReadSome(&out));

  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  testee_.AddChunk(base::make_unique<StringChunk>("test2"));

  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(5, testee_.FreeAtMostNBytes(7));

  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  testee_.AddChunk(base::make_unique<StringChunk>("test2"));
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(5, testee_.ReadSome(&out));
  EXPECT_EQ(25, testee_.FreeAsMuchAsPossible());
  EXPECT_EQ(0, testee_.offset());
  EXPECT_EQ(5, testee_.size());
}

}  // namespace io
