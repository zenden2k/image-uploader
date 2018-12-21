#include <gtest/gtest.h>

#include "Core/Utils/StringUtils.h"

class StringUtilsTest : public ::testing::Test {

};

using namespace IuStringUtils;

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