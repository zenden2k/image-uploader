#ifndef IU_FUNC_DEFAULTLOGGER_H
#define IU_FUNC_DEFAULTLOGGER_H

#pragma once

#include "Core/Logging/Logger.h"

class DefaultLogger : public ILogger {
public:
    virtual void write(LogMsgType MsgType, const std::string&  Sender, const std::string&  Msg, const std::string&  Info) override;
    virtual void write(LogMsgType MsgType, const wchar_t*  Sender, const wchar_t*   Msg, const wchar_t*  Info) override;
};

#endif