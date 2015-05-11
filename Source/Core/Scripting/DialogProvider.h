#ifndef IU_CORE_SCRIPTING_DIALOGPROVIDER_H
#define IU_CORE_SCRIPTING_DIALOGPROVIDER_H

#pragma once

#include <string>

class NetworkClient;

class IDialogProvider {
public:
    virtual ~IDialogProvider() {};
    virtual std::string askUserCaptcha(NetworkClient *nm, const std::string& url) = 0;
    virtual std::string inputDialog(const std::string& text, const std::string& defaultValue) = 0;
};

#endif