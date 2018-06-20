#include <gtest/gtest.h>

#include "Func/CmdLine.h"
#include "Func/WinUtils.h"

class CmdLineTest : public ::testing::Test {

};

TEST_F(CmdLineTest, Parse)
{
    CCmdLine cl;
    cl.Parse(_T("\"C:\\Program Files(x86)\\Image Uploader\\Image Uploader.exe\" /fromcontextmenu /imagesonly /upload  \"C:\\Users\\user\\Pictures\\generate_implementation.gif\" \"C:\\Users\\user\\Pictures\\iu buffer.gif\""));
    EXPECT_EQ(6, cl.GetCount());
    EXPECT_STREQ(_T("C:\\Program Files(x86)\\Image Uploader\\Image Uploader.exe"), cl[0]);
    EXPECT_STREQ(_T("/fromcontextmenu"), cl[1]);
    EXPECT_STREQ(_T("/imagesonly"), cl[2]);
    EXPECT_STREQ(_T("/upload"), cl[3]);
    EXPECT_STREQ(_T("C:\\Users\\user\\Pictures\\generate_implementation.gif"), cl[4]);
    EXPECT_STREQ(_T("C:\\Users\\user\\Pictures\\iu buffer.gif"), cl[5]);
    EXPECT_STREQ(_T("C:\\Program Files(x86)\\Image Uploader\\Image Uploader.exe"), cl.ModuleName());
    // OnlyParams could be not the same as params which were passed to program
    EXPECT_STREQ(_T("/fromcontextmenu /imagesonly /upload C:\\Users\\user\\Pictures\\generate_implementation.gif \"C:\\Users\\user\\Pictures\\iu buffer.gif\""), cl.OnlyParams());
    cl.Parse(_T(""));
    EXPECT_EQ(1, cl.GetCount());
    size_t index = cl.AddParam(_T("test"));
    EXPECT_EQ(1, index);
    EXPECT_STREQ(_T("test"), cl[1]);
    EXPECT_STREQ(_T(""), cl.ModuleName());

    CCmdLine cl2;
    CString moduleExt = WinUtils::GetFileExt(cl2.ModuleName());
    EXPECT_STREQ(_T("exe"), moduleExt);
}
