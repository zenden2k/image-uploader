#ifndef SERVERSCHECKER_SERVERSCHECKERSETTINGS_H
#define SERVERSCHECKER_SERVERSCHECKERSETTINGS_H

#include <string>
#include "atlheaders.h"

class BasicSettings;

namespace ServersListTool {

class ServersCheckerSettings {
public:
    std::string testFileName, testUrl;
    enum UseProxyEnum { kNoProxy = 0, kUserProxy, kSystemProxy } useProxy;
    enum ProxyType {
        ptHttp = 0, ptSocks4, ptSocks4a, ptSocks5, ptSocks5Dns
    } proxyType;
    std::string proxyAddress;
    int proxyPort;

    ServersCheckerSettings();
    bool loadFromFile(const std::string& fileName);
    bool saveToFile(const std::string& fileName = std::string()) const;
    void copySettings(BasicSettings* dest);
};

}
#endif