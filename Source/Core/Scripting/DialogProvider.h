#ifndef IU_CORE_SCRIPTING_DIALOGPROVIDER_H
#define IU_CORE_SCRIPTING_DIALOGPROVIDER_H

#pragma once

#include <string>
#include <mutex>

class NetworkClient;

class IDialogProvider {
public:
    virtual ~IDialogProvider() {};
    virtual std::string askUserCaptcha(NetworkClient *nm, const std::string& url) = 0;
    virtual std::string inputDialog(const std::string& text, const std::string& defaultValue) = 0;
    std::mutex& getMutex() { return dialogMutex_; }
protected:
    std::mutex dialogMutex_;
};

#endif