#define __STDC_LIMIT_MACROS
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <stdint.h>
#include <string>
//#include <limits.h>

#include <inttypes.h>
#include <gtest/gtest.h>
#include "Core/Utils/CoreUtils.h"
#ifdef _WIN32
#include <WinSock.h>
#endif
#include "Core/Utils/CryptoUtils.h"

class CoreUtilsTest : public ::testing::Test {
};
 using namespace IuCoreUtils;

const char * constSizeFileName = "TestData/file_with_const_size.png";
int64_t contSizeFileSize = 14830;

TEST_F(CoreUtilsTest, ExtractFilePath) 
{
    std::string fileName = "c:\\Program Files (x86)\\Image Uploader\\Image Uploader.exe";
    std::string result = ExtractFilePath(fileName);
    EXPECT_EQ("c:\\Program Files (x86)\\Image Uploader\\", result);
    result = ExtractFilePath("c:/Program Files (x86)\\Image Uploader/ExplorerIntegration64.dll");
    EXPECT_EQ("c:/Program Files (x86)\\Image Uploader/", result);
    result = ExtractFilePath("avcodec-56.dll");
    EXPECT_EQ("", result);
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


TEST_F(CoreUtilsTest, ExtractFileNameFromUrl)
{
    std::string result = ExtractFileNameFromUrl("http://www.directupload.net/file/d/3970/vmtbrege_jpg.htm");
    EXPECT_EQ("vmtbrege_jpg.htm", result);
    result = ExtractFileNameFromUrl("http://www.directupload.net/file/d/3970/vmtbrege_jpg.html?test=124&grgrg=54+5435");
    EXPECT_EQ("vmtbrege_jpg.html", result);

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
    EXPECT_EQ("-2147483648", int64_tToString(std::numeric_limits<int>::min()));
    EXPECT_EQ(int64_tToString(INT64_MAX), "9223372036854775807");
    EXPECT_EQ("-9223372036854775808", int64_tToString(INT64_MIN));
    EXPECT_EQ("-9223372036854775807", int64_tToString(INT64_MIN + 1) );
   
    EXPECT_EQ(int64_tToString(0), "0");
}


TEST_F(CoreUtilsTest, stringToint64_t) 
{
    EXPECT_TRUE(stringToInt64("9223372036854775807")== 9223372036854775807);
    EXPECT_TRUE(stringToInt64("-9223372036854775808")== -9223372036854775808);
    EXPECT_TRUE(stringToInt64("0")== 0);
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


TEST_F(CoreUtilsTest, Utf8ToWstring)
{
    std::wstring result = Utf8ToWstring("\x49\xC3\xB1\x74\xC3\xAB\x72\x6E\xC3\xA2\x74\x69\xC3\xB4\x6E\xC3\xA0\x6C\x69\x7A\xC3\xA6\x74\x69\xC3\xB8\x6E");
    EXPECT_EQ(result, L"I\u00F1t\u00EBrn\u00E2ti\u00F4n\u00E0liz\u00E6ti\u00F8n");
    result = Utf8ToWstring("");
    EXPECT_EQ(result, L"");
    result = Utf8ToWstring("test test");
    EXPECT_EQ(result, L"test test");
}

TEST_F(CoreUtilsTest, getFileSize)
{
    int64_t size = IuCoreUtils::getFileSize("not_existing_file22342343");
    EXPECT_EQ(size, -1);
    size = IuCoreUtils::getFileSize(constSizeFileName);
    EXPECT_EQ(size, contSizeFileSize);
}


TEST_F(CoreUtilsTest, copyFile)
{
    const char * destFile = "TestData/AnotherFolder/new_file.png";
    bool res = IuCoreUtils::copyFile(constSizeFileName, destFile, true);
    EXPECT_EQ(res, true);
    EXPECT_EQ(IuCoreUtils::FileExists(destFile), true);
    int64_t size = IuCoreUtils::getFileSize(destFile);
    EXPECT_EQ(size, contSizeFileSize);
}

#ifdef _WIN32
TEST_F(CoreUtilsTest, gettimeofday)
{
    struct timeval tp;
    gettimeofday(&tp, NULL);
    int64_t curTime = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    gettimeofday(&tp, NULL);
    int64_t curTime2 = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    EXPECT_GE(curTime2, curTime2);
}
#endif

TEST_F(CoreUtilsTest, ReadUtf8TextFile)
{
    std::string res;
    //ReadUtf8TextFile("d:\\Video\\Video\\Movies\\Laskovyy.may.2009.BDRip.avi", res);

    //EXPECT_EQ(size, );
}

TEST_F(CoreUtilsTest, GetFileContents)
{
    std::string data = GetFileContents(constSizeFileName);
    EXPECT_EQ(contSizeFileSize, data.size());
    std::string hash = IuCoreUtils::CryptoUtils::CalcMD5Hash(&data[0], data.size());
    EXPECT_EQ("ebbd98fc18bce0e9dd774f836b5c3bf8", hash);

    std::string data2 = GetFileContents("j:\\Video\\Movies\\Sem'janin.2000.HDRip.avi");
}
