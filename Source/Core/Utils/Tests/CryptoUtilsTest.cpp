#include <gtest/gtest.h>
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Tests/TestHelpers.h"

class CryptoUtilsTest : public ::testing::Test {
public:

    CryptoUtilsTest() : constSizeFileName(TestHelpers::resolvePath("file_with_const_size.png")) {

    }
protected:
    const std::string constSizeFileName;
};

using namespace IuCoreUtils::CryptoUtils;

TEST_F(CryptoUtilsTest, CalcMD5HashFromString)
{
    std::string result = CalcMD5HashFromString("zenden");
    EXPECT_EQ("9bdfa43c0351de396a363848380a99b1", result);
}

TEST_F(CryptoUtilsTest, CalcSHA1HashFromString)
{
    std::string result = CalcSHA1HashFromString("zenden");
    EXPECT_EQ("dae26f5db3ecb29fd0282b712d619bbb518a4201", result);
}

TEST_F(CryptoUtilsTest, CalcHMACSHA1HashFromString)
{
    std::string result = CalcHMACSHA1HashFromString("someKey", "zenden", true);
    EXPECT_EQ("vf8nxw3pAU1L+yqK1T8exEi2kJA=", result);
    result = CalcHMACSHA1HashFromString("f#0{s^44a\"$>tB3zeIM6", "zenden2k", false);
    EXPECT_EQ("6e9b7ea9b590ffd406d6d8b2dca4c0e85cde9556", result);
}

TEST_F(CryptoUtilsTest, CalcMD5HashFromFile)
{
    std::string result = CalcMD5HashFromFile(constSizeFileName);
    EXPECT_EQ("ebbd98fc18bce0e9dd774f836b5c3bf8", result);
}


TEST_F(CryptoUtilsTest, CalcSHA1HashFromFile)
{
    std::string result = CalcSHA1HashFromFile(constSizeFileName);
    EXPECT_EQ("9eaa90959561639b4f1abc4914ce7b8943e0351c", result);
}

TEST_F(CryptoUtilsTest, Base64Encode)
{
    EXPECT_EQ("QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGU=", Base64Encode("Base64 is a generic term for a number of similar encoding schemes that encode"));
    EXPECT_EQ("", Base64Encode(""));
}

TEST_F(CryptoUtilsTest, Base64Decode)
{
    EXPECT_EQ("Base64 is a generic term for a number of similar encoding schemes that encode", Base64Decode("QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciBhIG51bWJlciBvZiBzaW1pbGFyIGVuY29kaW5nIHNjaGVtZXMgdGhhdCBlbmNvZGU="));
    EXPECT_EQ("", Base64Decode(""));
}
