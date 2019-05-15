#ifndef IU_FUNC_DEFAULTLOGGER_H
#define IU_FUNC_DEFAULTLOGGER_H

#pragma once

#include "Core/Logging/Logger.h"

class ConsoleLogger : public ILogger {
public:
    virtual void write(LogMsgType MsgType, const std::string&  FileName, const std::string&  Sender, const std::string&  Msg, const std::string&  Info) override;
#ifdef _WIN32
    virtual void write(LogMsgType MsgType, const wchar_t*  FileName, const wchar_t*  Sender, const wchar_t*   Msg, const wchar_t*  Info) override;
#endif
};

#endif