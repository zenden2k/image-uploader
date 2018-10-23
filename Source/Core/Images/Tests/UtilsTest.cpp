#include <gtest/gtest.h>

#include "Core/Images/Utils.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/GdiPlusInitializer.h"
#include "Func/WinUtils.h"
#include "Core/Utils/CryptoUtils.h"

class UtilsTest : public ::testing::Test {
private:
    GdiPlusInitializer init;
public:

    void waitFormatAvailable() {
        bool isClipboardOpened = OpenClipboard(0) != FALSE;
        ASSERT_TRUE(isClipboardOpened);
        DWORD startTime = ::GetTickCount();
        while (IsClipboardFormatAvailable(CF_UNICODETEXT) == FALSE) {
            if (::GetTickCount() - startTime > 1000) {
                ASSERT_TRUE(false);
                break;
            }
        }
        CloseClipboard();
    }
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

    {
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

    {
        Gdiplus::Bitmap* thumb4 = GetThumbnail(L"TestData/Images/poroshok.webp", 150, 120, &size);
        ASSERT_TRUE(thumb4 != nullptr);
        EXPECT_EQ(800, size.Width);
        EXPECT_EQ(322, size.Height);
        EXPECT_LE(thumb4->GetWidth(), 150u);
        EXPECT_LE(thumb4->GetHeight(), 120u);
        /*Gdiplus::Color color;
        thumb4->GetPixel(0, 0, &color);
        EXPECT_EQ(1485865579, color.GetValue());*/
        delete thumb4;
    }

     {
         Gdiplus::Bitmap* thumb4 = GetThumbnail(L"TestData/Images/animated-webp-supported.webp", 150, 120, &size);
         ASSERT_TRUE(thumb4 != nullptr);
         EXPECT_EQ(400, size.Width);
         EXPECT_EQ(400, size.Height);
         EXPECT_LE(thumb4->GetWidth(), 150u);
         EXPECT_LE(thumb4->GetHeight(), 120u);
         Gdiplus::Color color;
         thumb4->GetPixel(0, 0, &color);
         EXPECT_EQ(1811951973, color.GetValue());
         delete thumb4;
     }

     for (int i = 1; i <= 8; i++ ) {
         Gdiplus::Bitmap* thumb5 = GetThumbnail(L"TestData/Images/Landscape_" + WinUtils::IntToStr(i) + L".jpg", 150, 120, &size);
         ASSERT_TRUE(thumb5 != nullptr);
         EXPECT_EQ(1800, size.Width);
         EXPECT_EQ(1200, size.Height);
         EXPECT_LE(thumb5->GetWidth(), 150u);
         EXPECT_LE(thumb5->GetHeight(), 120u);
         Gdiplus::Color color;
         thumb5->GetPixel(100, 100, &color);
         EXPECT_EQ(4278190080, color.GetValue());
         delete thumb5;
     }

     for (int i = 1; i <= 8; i++) {
         Gdiplus::Bitmap* thumb6 = GetThumbnail(L"TestData/Images/Portrait_" + WinUtils::IntToStr(i) + L".jpg", 150, 120, &size);
         ASSERT_TRUE(thumb6 != nullptr);
         EXPECT_EQ(1200, size.Width);
         EXPECT_EQ(1800, size.Height);
         EXPECT_LE(thumb6->GetWidth(), 150u);
         EXPECT_LE(thumb6->GetHeight(), 120u);
         Gdiplus::Color color;
         thumb6->GetPixel(100, 100, &color);
         EXPECT_EQ(4278190080, color.GetValue());
         delete thumb6;
     }


}

TEST_F(UtilsTest, CopyBitmapToClipboardInDataUriFormat) {
    const char* fileName = "TestData/file_with_const_size.png";
    std::unique_ptr<Bitmap> bm(Bitmap::FromFile(U2W(fileName)));
    ASSERT_TRUE(bm != nullptr);
    bool res = CopyBitmapToClipboardInDataUriFormat(bm.get(), 1, 85); 
    EXPECT_TRUE(res);

    waitFormatAvailable();
    
    CString clipboardText;
    res = WinUtils::GetClipboardText(clipboardText, 0, true);
    ASSERT_TRUE(res);
    std::string clipboardTextA = W2U(clipboardText);
    std::string md5 = IuCoreUtils::CryptoUtils::CalcMD5HashFromString(clipboardTextA);
    EXPECT_EQ("7b0e5e0e312bfed196c117fe821a4e30", md5);

    
    // Test HTML format
    res = CopyBitmapToClipboardInDataUriFormat(bm.get(), 1, 85, true);
    EXPECT_TRUE(res);
    waitFormatAvailable();
    CString clipboardText2;
    res = WinUtils::GetClipboardText(clipboardText2, 0, true);
    ASSERT_TRUE(res);
    std::string clipboardTextA2 = W2U(clipboardText2);
    std::string md52 = IuCoreUtils::CryptoUtils::CalcMD5HashFromString(clipboardTextA2);
    EXPECT_EQ("88d4f8b907c9b4d596a920419ccdd6f4", md52);
}

TEST_F(UtilsTest, GetImageInfo) {
    ImageInfo ii = GetImageInfo(U2W("TestData/file_with_const_size.png"));
    EXPECT_EQ(251, ii.width);
    EXPECT_EQ(366, ii.height);
    ImageInfo ii2 = GetImageInfo(U2W("TestData/notexistingfile57345345.png"));
    EXPECT_EQ(0, ii2.width);
    EXPECT_EQ(0, ii2.height);
}

TEST_F(UtilsTest, ExUtilReadFile) {
    const char* fileName = "TestData/file_with_const_size.png";
    uint8_t* data;
    size_t size;
    EXPECT_TRUE(ExUtilReadFile(U2W(fileName), &data, &size));
    EXPECT_EQ(14830, size);
    std::string hash = IuCoreUtils::CryptoUtils::CalcMD5Hash(data, size);
    EXPECT_EQ("ebbd98fc18bce0e9dd774f836b5c3bf8", hash);

    delete[] data;
    data = nullptr; 
    size = 0;
    EXPECT_FALSE(ExUtilReadFile(L"TestData/notexistingfile5734533345.png", &data, &size));
    EXPECT_EQ(nullptr, data);
    EXPECT_EQ(0, size);

    data = nullptr;
    size = 0;

    EXPECT_TRUE(ExUtilReadFile(L"TestData/file_with_zero_size.dat", &data, &size));
    EXPECT_EQ(0, size);
    EXPECT_TRUE(data != nullptr);
    delete[] data;
}