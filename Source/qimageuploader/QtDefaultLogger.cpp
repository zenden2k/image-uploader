#include "QtDefaultLogger.h"

#include <QString>
#include "Gui/LogWindow.h"
#include "Core/CommonDefs.h"

QtDefaultLogger::QtDefaultLogger(LogWindow* logWindow) {
	logWindow_ = logWindow;
}


void QtDefaultLogger::write(LogMsgType MsgType, const std::string& Sender, const std::string& Msg, const std::string& Info) {
	logWindow_->writeLog(MsgType, U2Q(Sender), U2Q(Msg), U2Q(Info));
	//CLogWindow::WriteLog(MsgType, U2W(Sender), U2W(Msg), U2W(Info));
}

void QtDefaultLogger::write(LogMsgType MsgType, const wchar_t* Sender, const wchar_t* Msg, const wchar_t* Info) {
	logWindow_->writeLog(MsgType, QString::fromWCharArray(Sender), QString::fromWCharArray(Msg), QString::fromWCharArray(Info));
	//CLogWindow::WriteLog(MsgType, Sender, Msg, Info);
}