#include <gtest/gtest.h>
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CryptoUtils.h"

#define constSizeFileName "TestData/file_with_const_size.png"

class CryptoUtilsTest : public ::testing::Test {

};

using namespace IuCoreUtils::CryptoUtils;

TEST_F(CryptoUtilsTest, CalcMD5HashFromString)
{
    std::string result = CalcMD5HashFromString("zenden");
    EXPECT_EQ(result, "9bdfa43c0351de396a363848380a99b1");
}

TEST_F(CryptoUtilsTest, CalcSHA1HashFromString)
{
    std::string result = CalcSHA1HashFromString("zenden");
    EXPECT_EQ(result, "dae26f5db3ecb29fd0282b712d619bbb518a4201");
}

TEST_F(CryptoUtilsTest, CalcHMACSHA1HashFromString)
{
    std::string result = CalcHMACSHA1HashFromString("someKey", "zenden", true);
    EXPECT_EQ(result, "vf8nxw3pAU1L+yqK1T8exEi2kJA=");
    result = CalcHMACSHA1HashFromString("f#0{s^44a\"$>tB3zeIM6", "zenden2k", false);
    EXPECT_EQ(result, "6e9b7ea9b590ffd406d6d8b2dca4c0e85cde9556");
}

TEST_F(CryptoUtilsTest, CalcMD5HashFromFile)
{
    std::string result = CalcMD5HashFromFile(constSizeFileName);
    EXPECT_EQ(result, "ebbd98fc18bce0e9dd774f836b5c3bf8");
}


TEST_F(CryptoUtilsTest, CalcSHA1HashFromFile)
{
    std::string result = CalcSHA1HashFromFile(constSizeFileName);
    EXPECT_EQ(result, "9eaa90959561639b4f1abc4914ce7b8943e0351c");
}


