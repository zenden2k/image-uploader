#include "DefaultLogger.h"

#include "Gui/Dialogs/LogWindow.h"

DefaultLogger::DefaultLogger(CLogWindow* logWindow): logWindow_(logWindow) {
}

void DefaultLogger::write(LogMsgType MsgType, const std::string& Sender, const std::string& Msg, const std::string& Info) {
    logWindow_->WriteLog(MsgType, U2W(Sender), U2W(Msg), U2W(Info));
}

void DefaultLogger::write(LogMsgType MsgType, const wchar_t* Sender, const wchar_t* Msg, const wchar_t* Info) {
    logWindow_->WriteLog(MsgType, Sender, Msg, Info);
}