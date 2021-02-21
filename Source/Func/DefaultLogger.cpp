#include "DefaultLogger.h"

#include <boost/format.hpp>
#include "atlheaders.h"
#include "Gui/Dialogs/LogWindow.h"

DefaultLogger::DefaultLogger() {
}

void DefaultLogger::write(LogMsgType MsgType, const std::string& Sender, const std::string& Msg, const std::string& Info, const std::string&  FileName) {
    LogEntry entry;
    entry.MsgType = MsgType;
    entry.Msg = IuCoreUtils::Utf8ToWstring(Msg);
    entry.Info = IuCoreUtils::Utf8ToWstring(Info);
    entry.Sender = IuCoreUtils::Utf8ToWstring(Sender);
    entry.FileName = IuCoreUtils::Utf8ToWstring(FileName);

    SYSTEMTIME st;
    ::GetLocalTime(&st);

    entry.Time  = str(boost::wformat(L"%02d:%02d:%02d")% static_cast<int>(st.wHour) % static_cast<int>(st.wMinute) % static_cast<int>(st.wSecond));

    int itemIndex;
    {
        std::lock_guard<std::mutex> lk(entriesMutex_);
        entries_.push_back(entry);
        itemIndex = entries_.size() - 1;
    }
   
    for (auto* listener : listeners_) {
        listener->onItemAdded(itemIndex, entry);
    }
}

void DefaultLogger::write(LogMsgType MsgType, const wchar_t* Sender, const wchar_t* Msg, const wchar_t* Info, const wchar_t*  FileName) {
    LogEntry entry;
    entry.MsgType = MsgType;
    entry.Msg = Msg;
    entry.Info = Info;
    entry.Sender = Sender;
    entry.FileName = FileName;
    SYSTEMTIME st;
    ::GetLocalTime(&st);

    entry.Time = str(boost::wformat(L"%02d:%02d:%02d") % static_cast<int>(st.wHour) % static_cast<int>(st.wMinute) % static_cast<int>(st.wSecond));


    int itemIndex;
    {
        std::lock_guard<std::mutex> lk(entriesMutex_);
        entries_.push_back(entry);
        itemIndex = entries_.size() - 1;
    }


    for (auto* listener : listeners_) {
        listener->onItemAdded(itemIndex, entry);
    }
}

void DefaultLogger::addListener(Listener* listener) {
    listeners_.push_back(listener);
}

void  DefaultLogger::removeListener(Listener* listener) {
    for (int i = static_cast<int>(listeners_.size()) - 1; i >= 0; i--) {
        if (listeners_[i] == listener) {
            listeners_[i] = listeners_[listeners_.size() - 1];
            listeners_.pop_back();
            break;
        }
    }
}
size_t DefaultLogger::entryCount() const {
    return entries_.size();
}

void DefaultLogger::getEntry(int itemIndex, LogEntry* out) {
    std::lock_guard<std::mutex> lk(entriesMutex_);
    *out = entries_[itemIndex];
}

std::mutex& DefaultLogger::getEntryMutex() {
    return entriesMutex_;
}

std::vector<DefaultLogger::LogEntry>::const_iterator DefaultLogger::begin() const {
    return entries_.begin();
}

std::vector<DefaultLogger::LogEntry>::const_iterator DefaultLogger::end() const {
    return entries_.end();
}

void DefaultLogger::clear() {
    std::lock_guard<std::mutex> lk(entriesMutex_);
    entries_.clear();
}