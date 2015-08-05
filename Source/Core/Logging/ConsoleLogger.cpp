#include "ConsoleLogger.h"

#include <iostream>
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/ConsoleUtils.h"
#include <mutex>

void ConsoleLogger::write(LogMsgType MsgType, const std::string& Sender, const std::string& Msg, const std::string& Info) {
    std::lock_guard<std::mutex> guard(ConsoleUtils::instance()->getOutputMutex());
//#ifdef _WIN32
    std::wcerr << IuCoreUtils::Utf8ToWstring(Msg) << std::endl;;
/*#else
    std::cerr << IuCoreUtils::Utf8ToSystemLocale(Msg) << std::endl;
#endif*/
}

#ifdef _WIN32
void ConsoleLogger::write(LogMsgType MsgType, const wchar_t* Sender, const wchar_t* Msg, const wchar_t* Info) {
    std::lock_guard<std::mutex> guard(ConsoleUtils::instance()->getOutputMutex());
    std::cerr << ( MsgType == logError ? "error" : "warning" ) << " : ";
//#ifdef _WIN32
    std::wcerr << Msg << std::endl;
/*#else
    std::cerr << IuCoreUtils::Utf8ToSystemLocale(IuCoreUtils::WstringToUtf8(Msg)) << std::endl;
#endif*/
}

#endif
