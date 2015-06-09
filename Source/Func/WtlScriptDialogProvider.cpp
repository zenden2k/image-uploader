#include "WtlScriptDialogProvider.h"
#include <Gui/Dialogs/InputDialog.h>
#include "WinUtils.h"
#include "IuCommonFunctions.h"
#include "Core/Network/NetworkClient.h"

std::string WtlScriptDialogProvider::askUserCaptcha(NetworkClient* nm, const std::string& url) {   
    std::lock_guard<std::mutex> guard(dialogMutex_);
    CString wFileName = WinUtils::GetUniqFileName(IuCommonFunctions::IUTempFolder + Utf8ToWstring("captcha").c_str());

    nm->setOutputFile(IuCoreUtils::WstringToUtf8((const TCHAR*)wFileName));
    if (!nm->doGet(url))
        return "";
    CInputDialog dlg(_T("Image Uploader"), TR("Please enter the text you see in the image:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()), wFileName);
    nm->setOutputFile("");
    if (dlg.DoModal() == IDOK)
        return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
    return "";
}

std::string WtlScriptDialogProvider::inputDialog(const std::string& text, const std::string& defaultValue) {
    std::lock_guard<std::mutex> guard(dialogMutex_);
    CInputDialog dlg(_T("Image Uploader"), Utf8ToWCstring(text), Utf8ToWCstring(defaultValue));

    if (dlg.DoModal(GetActiveWindow()) == IDOK) {
        return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
    }
    return "";
}