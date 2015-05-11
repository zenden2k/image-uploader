#include "ConsoleLogger.h"

#include "Gui/Dialogs/LogWindow.h"

void ConsoleLogger::write(LogMsgType MsgType, const std::string& Sender, const std::string& Msg, const std::string& Info) {
//#ifdef _WIN32
    std::wcerr << IuCoreUtils::Utf8ToWstring(Msg) << std::endl;;
/*#else
    std::cerr << IuCoreUtils::Utf8ToSystemLocale(Msg) << std::endl;
#endif*/
}

void ConsoleLogger::write(LogMsgType MsgType, const wchar_t* Sender, const wchar_t* Msg, const wchar_t* Info) {
    std::cerr << ( MsgType == logError ? "error" : "warning" ) << " : ";
//#ifdef _WIN32
    std::wcerr << Msg << std::endl;
/*#else
    std::cerr << IuCoreUtils::Utf8ToSystemLocale(IuCoreUtils::WstringToUtf8(Msg)) << std::endl;
#endif*/
}