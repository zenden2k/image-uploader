#include "Helpers.h"

#include "Func/WinUtils.h"
#include "3rdpart/GdiPlusH.h"

namespace ServersListTool {
namespace Helpers {

CString MyBytesToString(int64_t nBytes)
{
    return IuCoreUtils::fileSizeToString(nBytes).c_str();
}

CString GetFileInfo(CString fileName, MyFileInfo* mfi)
{
    CString result;
    int fileSize = static_cast<int>(IuCoreUtils::getFileSize(W2U(fileName)));
    result = MyBytesToString(fileSize) + _T("(") + WinUtils::IntToStr(fileSize) + _T(" bytes);");
    CString mimeType = IuCoreUtils::GetFileMimeType(W2U(fileName)).c_str();
    result += mimeType + _T(";");
    if (mfi) mfi->mimeType = mimeType;
    if (mimeType.Find(_T("image/")) >= 0) {
        Gdiplus::Image pic(fileName);
        int width = pic.GetWidth();
        int height = pic.GetHeight();
        if (mfi) {
            mfi->width = width;
            mfi->height = height;
        }
        result += WinUtils::IntToStr(width) + _T("x") + WinUtils::IntToStr(height);
    }
    return result;
}

}
}