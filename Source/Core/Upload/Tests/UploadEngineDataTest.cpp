#include <gtest/gtest.h>

#include "Core/Upload/UploadEngine.h"
#include "Tests/TestHelpers.h"

class UploadEngineDataTest : public ::testing::Test {
public:
    UploadEngineDataTest()
        : fileName(TestHelpers::resolvePath("servers.xml"))
    {

    }
protected:
    const std::string fileName;
};

TEST_F(UploadEngineDataTest, supportsFileFormat) {
    const auto TEST_FILE_SIZE = 50000;
    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeFileServer;

    FileFormatGroup gr;
    FileFormat format1;
    format1.MimeTypes = { "video/mp4" };
    format1.FileNameWildcards = { "*.mp4" };
    gr.Formats.push_back(format1);
    ued.SupportedFormatGroups.push_back(gr);
    EXPECT_TRUE(ued.supportsFileFormat("test.mp4", "video/mp4", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("test.mp3", "video/mp4", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("test.mp4", "application/octet-stream", TEST_FILE_SIZE, false));
}

TEST_F(UploadEngineDataTest, supportsFileFormatSimpleOne) {
    const auto TEST_FILE_SIZE = 50000;
    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeFileServer;

    FileFormatGroup gr;
    FileFormat format1;
    format1.MimeTypes = { "video/mp4" };
    format1.FileNameWildcards = { "*" };
    gr.Formats.push_back(format1);
    ued.SupportedFormatGroups.push_back(gr);
    EXPECT_TRUE(ued.supportsFileFormat("test.mp4", "video/mp4", TEST_FILE_SIZE, false));
    EXPECT_TRUE(ued.supportsFileFormat("test.txt", "video/mp4", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("test.mp4", "application/octet-stream", TEST_FILE_SIZE, false));
}

TEST_F(UploadEngineDataTest, supportsFileFormatAnotherSimple)
{
    const auto TEST_FILE_SIZE = 50000;
    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeFileServer;

    FileFormatGroup gr;
    FileFormat format1;
    format1.MimeTypes = { "*" };
    format1.FileNameWildcards = { "*.mp3" };
    gr.Formats.push_back(format1);
    ued.SupportedFormatGroups.push_back(gr);
    EXPECT_TRUE(ued.supportsFileFormat("test.mp3", "audio/mpeg", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("test.txt", "audio/mpeg", TEST_FILE_SIZE, false));
    EXPECT_TRUE(ued.supportsFileFormat("test2.mp3", "application/octet-stream", TEST_FILE_SIZE, false));
}

TEST_F(UploadEngineDataTest, supportsFileFormatEmptyArgs) {
    const auto TEST_FILE_SIZE = 900000;
    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeFileServer;

    FileFormatGroup gr;
    FileFormat format1;
    format1.MimeTypes = { "audio/mpeg" };
    format1.FileNameWildcards = { "*.mp3" };
    gr.Formats.push_back(format1);
    ued.SupportedFormatGroups.push_back(gr);
    EXPECT_FALSE(ued.supportsFileFormat("", "audio/mpeg", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("", "audio/mpeg", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("", "application/octet-stream", TEST_FILE_SIZE, false));
}

TEST_F(UploadEngineDataTest, supportsFileFormatMultipleMimeTypes) {
    const auto TEST_FILE_SIZE = 900000;
    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeFileServer;

    FileFormatGroup gr;
    FileFormat format1;
    format1.MimeTypes = { "audio/mpeg", "video/*" };
    format1.FileNameWildcards = { "*" };
    gr.Formats.push_back(format1);
    ued.SupportedFormatGroups.push_back(gr);
    EXPECT_TRUE(ued.supportsFileFormat("test.mp3", "audio/mpeg", TEST_FILE_SIZE, false));
    EXPECT_TRUE(ued.supportsFileFormat("test.mov", "video/quicktime", TEST_FILE_SIZE, false));
    EXPECT_FALSE(ued.supportsFileFormat("test.mp3", "application/octet-stream", TEST_FILE_SIZE, false));
    EXPECT_TRUE(ued.supportsFileFormat("test.mpg", "video/mpeg", TEST_FILE_SIZE, false));
}

TEST_F(UploadEngineDataTest, supportsFileFormatAuthorized) {
    // Prepare CUploadEngineData instance
    CUploadEngineData ued;
    ued.Name = "test server";
    ued.TypeMask = CUploadEngineData::TypeFileServer;

    FileFormatGroup gr;
    FileFormat format1;
    format1.MimeTypes = { "audio/mpeg", "video/*" };
    format1.FileNameWildcards = { "*" };
    gr.Authorized = true;
    gr.MaxFileSize = 20000000;
    gr.Formats.push_back(format1);

    ued.SupportedFormatGroups.push_back(gr);

    FileFormatGroup gr2;
    FileFormat format2;
    format2.MimeTypes = { "image/jpeg" };
    format2.FileNameWildcards = { "*.jpg", "*.jpeg" };
    FileFormat format3;
    format3.MimeTypes = { "image/png" };
    format3.FileNameWildcards = { "*.png" };
    gr2.Authorized = false;
    gr2.MaxFileSize = 50000;
    gr2.Formats.push_back(format2);
    gr2.Formats.push_back(format3);
    ued.SupportedFormatGroups.push_back(gr2);
    EXPECT_TRUE(ued.supportsFileFormat("cool_file1.png", "image/png", 25000, false));
    EXPECT_FALSE(ued.supportsFileFormat("cool_file2.png", "image/png", 100000, false));
    EXPECT_FALSE(ued.supportsFileFormat("cool_file3.jpg", "image/jpeg", 100000, true));
    EXPECT_TRUE(ued.supportsFileFormat("cool_file4.mp3", "audio/mpeg", 900000, true));
   
    EXPECT_TRUE(ued.supportsFileFormat("test5.mp3", "audio/mpeg", 900000, true));
    EXPECT_TRUE(ued.supportsFileFormat("test6.mpg", "video/mpeg", 920000, true));
    EXPECT_TRUE(ued.supportsFileFormat("test6.mpg", "video/quicktime", 920000, true));
    EXPECT_FALSE(ued.supportsFileFormat("test6.mpg", "video/quicktime", 20000001, true));
    EXPECT_FALSE(ued.supportsFileFormat("test6.mpg", "video/quicktime", 920000, false));
}
