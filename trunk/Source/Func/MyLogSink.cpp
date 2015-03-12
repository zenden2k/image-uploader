#include "MyLogSink.h"
#include <Gui/Dialogs/LogWindow.h>
#include <Core/Utils/CoreUtils.h>

void MyLogSink::send(google::LogSeverity severity, const char* full_filename, const char* base_filename, int line, const struct ::tm* tm_time, const char* message, size_t message_len)
{
	std::string sender = base_filename;
	sender += ":"+IuCoreUtils::int64_tToString(line);
	CString msg = IuCoreUtils::Utf8ToWstring(message).c_str();
	WriteLog(severity ==google::GLOG_ERROR ? logError : logWarning,  IuCoreUtils::Utf8ToWstring(sender).c_str(),msg );
}

