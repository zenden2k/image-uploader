#ifndef IU_FUNC_DEFAULTLOGGER_H
#define IU_FUNC_DEFAULTLOGGER_H

#pragma once

#include "atlheaders.h"
#include "Core/Logging/Logger.h"
#include <deque>
#include <mutex>

class CLogWindow;

class DefaultLogger : public ILogger {
public:
    struct LogEntry {
        LogMsgType MsgType;
        CString Sender;
        CString Msg;
        CString Info;
        CString FileName;
        CString Time;
    };
    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void onItemAdded(int rowIndex, const LogEntry&) = 0;
    };
    DefaultLogger();
    void addListener(Listener* listener);
    void removeListener(Listener* listener);
    size_t entryCount() const;
    void clear();
    void getEntry(int itemIndex, LogEntry* out);
    std::mutex& getEntryMutex();
    void write(LogMsgType MsgType, const std::string&  Sender, const std::string&  Msg, const std::string&  Info, const std::string&  FileName) override;
    void write(LogMsgType MsgType, const wchar_t*  Sender, const wchar_t*   Msg, const wchar_t*  Info, const wchar_t*  FileName) override;
    std::vector<LogEntry>::const_iterator begin() const;
    std::vector<LogEntry>::const_iterator end() const;
private:
    CLogWindow* logWindow_;
    std::vector<LogEntry> entries_;
    std::vector<Listener*> listeners_;
    std::mutex entriesMutex_;
};

#endif