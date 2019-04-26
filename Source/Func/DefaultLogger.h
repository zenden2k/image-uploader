#ifndef IU_FUNC_DEFAULTLOGGER_H
#define IU_FUNC_DEFAULTLOGGER_H

#pragma once

#include "Core/Logging/Logger.h"

class CLogWindow;

class DefaultLogger : public ILogger {
public:
    DefaultLogger(CLogWindow* logWindow);
    void write(LogMsgType MsgType, const std::string&  Sender, const std::string&  Msg, const std::string&  Info) override;
    void write(LogMsgType MsgType, const wchar_t*  Sender, const wchar_t*   Msg, const wchar_t*  Info) override;
private:
    CLogWindow* logWindow_;
};

#endif