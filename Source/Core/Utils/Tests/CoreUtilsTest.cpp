#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <string>
//#include <limits.h>

#include <inttypes.h>
#include <gtest/gtest.h>
#include "Core/Utils/CoreUtils.h"

class CoreUtilsTest : public ::testing::Test {
};
 using namespace IuCoreUtils;

TEST_F(CoreUtilsTest, ExtractFilePath) 
{
    std::string fileName = "c:\\Program Files (x86)\\Image Uploader\\Image Uploader.exe";
    std::string result = ExtractFilePath(fileName);
    EXPECT_EQ(result, "c:\\Program Files (x86)\\Image Uploader\\");
    result = ExtractFilePath("c:/Program Files (x86)\\Image Uploader/ExplorerIntegration64.dll");
    EXPECT_EQ(result, "c:/Program Files (x86)\\Image Uploader/");
    result = ExtractFilePath("avcodec-56.dll");
    EXPECT_EQ(result, "");
}


TEST_F(CoreUtilsTest, ExtractFileName) 
{
    std::string fileName = "c:\\Program Files (x86)\\Image Uploader\\Image Uploader.exe";
    std::string result = ExtractFileName(fileName);
    EXPECT_EQ(result, "Image Uploader.exe");
    result = ExtractFileName("c:/Program Files (x86)\\Image Uploader/ExplorerIntegration64.dll");
    EXPECT_EQ(result, "ExplorerIntegration64.dll");
    result = ExtractFileName("avcodec-56.dll");
    EXPECT_EQ(result, "avcodec-56.dll");
}
    
TEST_F(CoreUtilsTest, ExtractFileExt) 
{
    std::string fileName = "c:\\Program Files (x86)\\Image Uploader\\Image Uploader.exe";
    std::string result = ExtractFileExt(fileName);
    EXPECT_EQ(result, "exe");
    result = ExtractFileExt("c:/Program Files (x86)\\Image Uploader/ExplorerIntegration64.dll");
    EXPECT_EQ(result, "dll");
    result = ExtractFileExt("c:\\Program Files .(x86)\\Image Uploader.exe\\Image Uploader");
    EXPECT_EQ(result, "");
    result = ExtractFileExt("archive.tar.gz");
    EXPECT_EQ(result, "gz");
}

TEST_F(CoreUtilsTest, ExtractFileNameNoExt) 
{
    std::string fileName = "c:\\Program Files (x86)\\Image Uploader\\Image Uploader.exe";
    std::string result = ExtractFileNameNoExt(fileName);
    EXPECT_EQ(result, "Image Uploader");
    result = ExtractFileNameNoExt("c:/Program Files (x86)\\Image Uploader/ExplorerIntegration64.dll");
    EXPECT_EQ(result, "ExplorerIntegration64");
    result = ExtractFileNameNoExt("c:\\Program Files .(x86)\\Image Uploader.exe\\Image Uploader");
    EXPECT_EQ(result, "Image Uploader");
    result = ExtractFileNameNoExt("archive.tar.gz");
    EXPECT_EQ(result, "archive.tar");
}

TEST_F(CoreUtilsTest, toString) 
{
   // std::string result = toString(100);

    EXPECT_EQ(toString(100), "100");
    EXPECT_EQ(toString(2147483647), "2147483647");
    EXPECT_EQ(toString(-2147483646), "-2147483646");
    EXPECT_EQ(toString(0), "0");
    EXPECT_EQ(toString((unsigned int)0xffffffff), "4294967295");
    EXPECT_EQ(toString(100.5, 2), "100.50");
    EXPECT_EQ(toString(100.54534, 0), "101");
}


TEST_F(CoreUtilsTest, int64_tToString) 
{
    EXPECT_EQ(int64_tToString(INT64_MAX), "9223372036854775807");
    //EXPECT_EQ(int64_tToString(INT64_MIN), "-9223372036854775808");
    EXPECT_EQ(int64_tToString(INT64_MIN+1), "-9223372036854775807");
    
    EXPECT_EQ(int64_tToString(0), "0");
}


TEST_F(CoreUtilsTest, stringToint64_t) 
{
    EXPECT_TRUE(stringToint64_t("9223372036854775807")== 9223372036854775807);
    EXPECT_TRUE(stringToint64_t("-9223372036854775808")== -9223372036854775808);
    EXPECT_TRUE(stringToint64_t("0")== 0);
}

TEST_F(CoreUtilsTest, GetDefaultExtensionForMimeType) 
{
    EXPECT_EQ(GetDefaultExtensionForMimeType("image/gif"), "gif");
    EXPECT_EQ(GetDefaultExtensionForMimeType("image/png"), "png");
    EXPECT_EQ(GetDefaultExtensionForMimeType("image/jpeg"), "jpg");
    EXPECT_EQ(GetDefaultExtensionForMimeType("application/unknown"), "");
}

TEST_F(CoreUtilsTest, fileSizeToString) 
{
    EXPECT_EQ(fileSizeToString(140*1024*1024), "140.0 MB");
    EXPECT_EQ(fileSizeToString(25.5*1024), "26 KB");
    EXPECT_EQ(fileSizeToString(0), "0 bytes");
}

