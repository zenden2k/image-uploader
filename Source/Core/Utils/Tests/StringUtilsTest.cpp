#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "Core/Utils/StringUtils.h"
#include "Core/3rdpart/xdgmime/ports/fnmatch.h"

using namespace IuStringUtils;

class StringUtilsTest : public ::testing::Test {
protected:
    void TestMatch(std::string_view pattern, std::string_view str, int opts, bool expected) {
        int result = PatternMatch(pattern, str, opts);
        if (expected) {
            EXPECT_EQ(0, result) << "Pattern: '" << pattern << "' String: '" << str << "' Flags: " << opts;
        } else {
            EXPECT_EQ(NoMatch, result) << "Pattern: '" << pattern << "' String: '" << str << "' Flags: " << opts;
        }
    }
};


using ::testing::ElementsAre;

TEST_F(StringUtilsTest, LenghtOfUtf8String)
{
    EXPECT_EQ(11, LengthOfUtf8String("Hello world"));
    EXPECT_EQ(12, LengthOfUtf8String("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82\x2C\x20\xD0\xBC\xD0\xB8\xD1\x80\x21"));
    EXPECT_EQ(0, LengthOfUtf8String(""));
}

TEST_F(StringUtilsTest, stricmp)
{
    EXPECT_EQ(0, IuStringUtils::stricmp("Hello world", "hello world"));
    EXPECT_GT(IuStringUtils::stricmp("Yandex", "google"), 0);
    EXPECT_LT(IuStringUtils::stricmp("some string", "test"), 0);
}

TEST_F(StringUtilsTest, SplitSV) {
    std::string test = "1,22,333,4,5";
    auto tokens = IuStringUtils::SplitSV(test, ",");
    EXPECT_THAT(tokens, ElementsAre("1", "22", "333", "4", "5"));
    std::string test2 = "Hello: World: 123";
    auto tokens2 = IuStringUtils::SplitSV(test2, ":", 2);
    EXPECT_THAT(tokens2, ElementsAre("Hello", " World: 123"));
    std::string test3 = "1,22,333;4,5";
    auto tokens3 = IuStringUtils::SplitSV(test3, ";,");
    EXPECT_THAT(tokens3, ElementsAre("1", "22", "333", "4", "5"));
}

TEST_F(StringUtilsTest, PatternMatch_BasicMatches) {
    TestMatch("abc", "abc", 0, true);
    TestMatch("a*c", "abc", 0, true);
    TestMatch("a?c", "abc", 0, true);
    TestMatch("a[bc]d", "abd", 0, true);
    TestMatch("a*b*c", "abc", 0, true);
    TestMatch("a***c", "abc", 0, true);
}

TEST_F(StringUtilsTest, PatternMatch_BasicMismatches) {
    TestMatch("abc", "abcd", 0, false);
    TestMatch("a*c", "ab", 0, false);
    TestMatch("a?c", "ac", 0, false);
    TestMatch("a[bc]d", "aed", 0, false);
}

TEST_F(StringUtilsTest, PatternMatch_CaseSensitivity) {
    TestMatch("abc", "ABC", 0, false);
    TestMatch("abc", "ABC", FoldCase, true);
    TestMatch("a*B", "aXb", FoldCase, true);
}

TEST_F(StringUtilsTest, PatternMatch_PathnameHandling) {
    TestMatch("a*b", "a/b", 0, true);
    TestMatch("a*b", "a/b", FileName, false);
    TestMatch("a?b", "a/b", FileName, false);
}
/*
TEST_F(StringUtilsTest, PeriodHandling) {
    TestMatch("*", ".hidden", 0, true);
    //TestMatch("*", ".hidden", Period, false);
    TestMatch("a*", "a/.hidden", Period | FileName, false);
}*/

TEST_F(StringUtilsTest, PatternMatch_EscapeHandling) {
    TestMatch("a\\*b", "a*b", 0, true);
    TestMatch("a\\*b", "a*b", NoEscape, false);
    TestMatch("a\\*b", "a\\*b", NoEscape, true);
}

/*TEST_F(StringUtilsTest, LeadingDirHandling) {
    TestMatch("abc*", "abc/def", 0, false);
    TestMatch("abc*", "abc/def", LeadingDir, true);
    TestMatch("abc*", "abcdef", LeadingDir, true);
}*/

TEST_F(StringUtilsTest, PatternMatch_EdgeCases) {
    TestMatch("", "", 0, true);
    TestMatch("*", "", 0, true);
    TestMatch("?", "", 0, false);
    TestMatch("[a]", "", 0, false);
    TestMatch("\\", "\\", NoEscape, true);
}

TEST_F(StringUtilsTest, PatternMatch_BracketExpressions) {
    TestMatch("[abc]", "a", 0, true);
    TestMatch("[!abc]", "d", 0, true);
    TestMatch("[a-c]", "b", 0, true);
    TestMatch("[!a-c]", "d", 0, true);
    TestMatch("[[a]", "[", 0, true);
}
