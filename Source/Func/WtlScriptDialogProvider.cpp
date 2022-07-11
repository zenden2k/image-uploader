#include "WtlScriptDialogProvider.h"

#include "Gui/Dialogs/InputDialog.h"
#include "Core/CommonDefs.h"
#include "WinUtils.h"
#include "Core/Network/NetworkClient.h"
#include "Core/AppParams.h"

std::string WtlScriptDialogProvider::askUserCaptcha(NetworkClient* nm, const std::string& url) {   
    std::lock_guard<std::mutex> guard(dialogMutex_);
    const CString wFileName = WinUtils::GetUniqFileName(AppParams::instance()->tempDirectoryW() + Utf8ToWstring("captcha").c_str());

    nm->setOutputFile(W2U(wFileName));
    if (!nm->doGet(url))
        return {};
    CInputDialog dlg(_T("Image Uploader"), TR("Please enter the text you see in the image:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()), wFileName);
    nm->setOutputFile({});
    IProgramWindow* window = ServiceLocator::instance()->programWindow();
    if (dlg.DoModal(window ? window->getHandle() : GetActiveWindow()) == IDOK)
        return W2U(dlg.getValue());
    return {};
}

std::string WtlScriptDialogProvider::inputDialog(const std::string& text, const std::string& defaultValue) {
    std::lock_guard<std::mutex> guard(dialogMutex_);
    CInputDialog dlg(_T("Image Uploader"), Utf8ToWCstring(text), Utf8ToWCstring(defaultValue));
    IProgramWindow* window = ServiceLocator::instance()->programWindow();
	
    if (dlg.DoModal(window? window->getHandle(): GetActiveWindow()) == IDOK) {
        return W2U(dlg.getValue());
    }
    return {};
}