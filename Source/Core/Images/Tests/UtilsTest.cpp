#include <gtest/gtest.h>

#include "Core/Images/Utils.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/GdiPlusInitializer.h"

class UtilsTest : public ::testing::Test {
private:
    GdiPlusInitializer init;
};

TEST_F(UtilsTest, BitmapFromMemory)
{
    const char* fileName = "TestData/file_with_const_size.png";
    int fileSize = static_cast<int>(IuCoreUtils::getFileSize(fileName));
    ASSERT_GT(fileSize, 0);
    uint8_t* data = new uint8_t[fileSize];
    FILE *f = IuCoreUtils::fopen_utf8(fileName, "rb");
    size_t bytesRead = fread(data, 1, fileSize, f);
    ASSERT_EQ(fileSize, bytesRead);
    fclose(f);
    Gdiplus::Bitmap* bm = BitmapFromMemory(data, fileSize);
    ASSERT_TRUE(bm != nullptr);
    EXPECT_EQ(366, bm->GetHeight());
    EXPECT_EQ(251, bm->GetWidth());
    Gdiplus::Color color;
    Gdiplus::Status status = bm->GetPixel(10, 10, &color);
    EXPECT_EQ(Gdiplus::Ok, status);
    EXPECT_EQ(16777215, color.GetValue());

    delete bm;
    delete[] data;
}

TEST_F(UtilsTest, GetThumbnail) {
    const char* fileName = "TestData/file_with_const_size.png";
    int fileSize = static_cast<int>(IuCoreUtils::getFileSize(fileName));
    ASSERT_GT(fileSize, 0);
    uint8_t* data = new uint8_t[fileSize];
    FILE *f = IuCoreUtils::fopen_utf8(fileName, "rb");
    size_t bytesRead = fread(data, 1, fileSize, f);
    ASSERT_EQ(fileSize, bytesRead);
    fclose(f);
    Gdiplus::Bitmap* bm = BitmapFromMemory(data, fileSize);
    ASSERT_TRUE(bm != nullptr);
    Gdiplus::Size size;
    Gdiplus::Bitmap* thumb = GetThumbnail(bm, 120, 100, &size);
    ASSERT_TRUE(thumb != nullptr);
    EXPECT_LE(thumb->GetWidth(), 120u);
    EXPECT_EQ(100, thumb->GetHeight());
    EXPECT_EQ(251, size.Width);
    EXPECT_EQ(366, size.Height);
    delete thumb;
    thumb = 0;

    Gdiplus::Bitmap* thumb2 = GetThumbnail(bm, 251, 366, &size);
    ASSERT_TRUE(thumb2 != nullptr);
  
    EXPECT_EQ(251, thumb2->GetWidth());
    EXPECT_EQ(366, thumb2->GetHeight());
    delete thumb2;
    thumb2 = nullptr;

    delete bm;
    delete data;

    Gdiplus::Bitmap* thumb3 = GetThumbnail(L"TestData/Images/exif-rgb-thumbnail.jpg", 150, 120, &size);
    ASSERT_TRUE(thumb3 != nullptr);
    EXPECT_EQ(293, size.Width);
    EXPECT_EQ(233, size.Height);
    EXPECT_LE(thumb3->GetWidth(), 150u);
    EXPECT_LE(thumb3->GetHeight(), 120u);
    Gdiplus::Color color;
    thumb3->GetPixel(0, 0, &color);
    EXPECT_EQ(3637300478, color.GetValue());
    delete thumb3;
}

TEST_F(UtilsTest, CopyBitmapToClipboardInDataUriFormat) {
    const char* fileName = "TestData/file_with_const_size.png";
    Bitmap* bm = Bitmap::FromFile(U2W(fileName));
    ASSERT_TRUE(bm != nullptr);
    //CopyBitmapToClipboardInDataUriFormat(bm, 0, 85);

}