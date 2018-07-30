#include "ConsoleScriptDialogProvider.h"

#include "Core/Network/NetworkClient.h"
#include "Core/Utils/DesktopUtils.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/ConsoleUtils.h"
#include <iostream>

std::string ConsoleScriptDialogProvider::askUserCaptcha(NetworkClient* nm, const std::string& url) {
    std::lock_guard<std::mutex> guard(ConsoleUtils::instance()->getOutputMutex());
    DesktopUtils::ShellOpenUrl(url);
    std::cerr << "Enter text from the image:" << std::endl;
//#ifdef _WIN32
    std::wstring result;
    std::wcin >> result;
    return IuCoreUtils::WstringToUtf8(result);
//#else
   /* std::string result;
    std::cin >> result;
    return result;*/
}

std::string ConsoleScriptDialogProvider::inputDialog(const std::string& text, const std::string& defaultValue) {
    std::lock_guard<std::mutex> guard(ConsoleUtils::instance()->getOutputMutex());
    std::string result;
    std::cerr << std::endl << text << std::endl;
    std::cin >> result;
    return result;
}
