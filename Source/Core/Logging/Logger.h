#ifndef IU_CORE_LOGGIN_LOGGER_H
#define IU_CORE_LOGGIN_LOGGER_H

#pragma once

enum LogMsgType { logError = 1, logWarning };

class ILogger {
public :
    virtual ~ILogger(){};
    virtual void write(LogMsgType MsgType, const std::string&  Sender, const std::string&  Msg, const std::string&  Info = "") = 0;
    virtual void write(LogMsgType MsgType, const wchar_t*  Sender, const wchar_t*   Msg, const wchar_t*  Info = L"") = 0;
};

#endif