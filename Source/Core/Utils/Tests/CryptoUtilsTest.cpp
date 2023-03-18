#include <gtest/gtest.h>
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Tests/TestHelpers.h"

class CryptoUtilsTest : public ::testing::Test {
public:

    CryptoUtilsTest() : constSizeFileName(TestHelpers::resolvePath("file_with_const_size.png")),
        zeroSizeFileName(TestHelpers::resolvePath("file_with_zero_size.dat"))
    {

    }
protected:
    const std::string constSizeFileName;
    const std::string zeroSizeFileName;
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

TEST_F(CryptoUtilsTest, CalcSHA1HashFromFileWithPrefix)
{
    std::string fileName = constSizeFileName;
    int64_t fileSize = IuCoreUtils::GetFileSize(fileName);
    std::string result = CalcSHA1HashFromFileWithPrefix(fileName, "mrCloud", std::to_string(fileSize));
    EXPECT_EQ("be933e5fa7236218736be5a538e559da15f5ab00", result);
}

TEST_F(CryptoUtilsTest, CalcSHA256HashFromString)
{
    std::string result = CalcSHA256HashFromString("zenden");
    EXPECT_EQ("f8b1f55e96be17bd326d178a472a00918d30f68604ce5ce2d5faf6cc3790eee6", result);
}

TEST_F(CryptoUtilsTest, CalcSHA256HashFromFile)
{
    std::string result = CalcSHA256HashFromFile(constSizeFileName);
    EXPECT_EQ("4a4c3b5200abfdcc89d67f0b8dbbe2da1ee91b49895e855256061222b41c8458", result);

    std::string result2 = CalcSHA256HashFromFile(constSizeFileName, 1000, 2000);
    EXPECT_EQ("e21595586706b01f90a3f78d1b49af9c7d2a58ed5613120c8a60fe109c14ff72", result2);
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

TEST_F(CryptoUtilsTest, Base64EncodeFile)
{
    {
        std::string output;
        bool res = Base64EncodeFile(constSizeFileName, output);
        EXPECT_TRUE(res);
        EXPECT_EQ("e0f0a6e5fcd8712fe470af8bc875d525", CalcMD5HashFromString(output));
    }
    {
        std::string output;
        bool res = Base64EncodeFile(zeroSizeFileName, output);
        EXPECT_TRUE(res);
        EXPECT_EQ("", output);
    }
    {
        std::string output;
        bool res = Base64EncodeFile(TestHelpers::resolvePath("utf8_text_file.txt"), output);
        EXPECT_TRUE(res);
        EXPECT_EQ("VGVzdCBmaWxlIHdpdGhvdXQgYm9tLiDQotC10YHRgtC+0LLRi9C5INGE0LDQudC7INCx0LXQtyBCT00u", output);
    }
    {
        std::string output;
        bool res = Base64EncodeFile("notexistingfile437859347650342643438095734", output);
        EXPECT_FALSE(res);
        EXPECT_TRUE(output.empty());
    }
    
}