#include <gtest/gtest.h>
#include <string>
#include <Core/Utils/CoreUtils.h>

class CoreUtilsTest : public ::testing::Test {
};

TEST_F(CoreUtilsTest, ExtractFilePath) 
{
    using namespace IuCoreUtils;
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
    using namespace IuCoreUtils;
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
    using namespace IuCoreUtils;
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
    using namespace IuCoreUtils;
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




