#include "ServersCheckerSettings.h"

#include "Func/WinUtils.h"
#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Settings/BasicSettings.h"

namespace ServersListTool {
   
ServersCheckerSettings::ServersCheckerSettings() {
    testFileName = W2U(WinUtils::GetAppFolder()) + "testfile.jpg";
    testUrl = "https://github.com/zenden2k/image-uploader/issues";
    useProxy = kNoProxy;
    proxyType = ptHttp;
    proxyPort = 8080;
}

bool ServersCheckerSettings::loadFromFile(const std::string& fileName) {
    SimpleXml xml;
    if (xml.LoadFromFile(fileName)) {
        SimpleXmlNode root = xml.getRoot("ServerListTool");
        std::string name = root.Attribute("FileName");
        if (!name.empty()) {
            testFileName = name;
        }
        std::string url = root.Attribute("URL");
        if (!url.empty()) {
            testUrl = url;
        }

        useProxy = static_cast<UseProxyEnum>(root.AttributeInt("UseProxy"));
        proxyType = static_cast<ProxyType>(root.AttributeInt("ProxyType"));
        proxyAddress = root.Attribute("ProxyAddress");
        int port = root.AttributeInt("ProxyPort");
        if (port) {
            proxyPort = port;
        }

        return true;
    }
    return false;
}

bool ServersCheckerSettings::saveToFile(const std::string& fileName) const {
    SimpleXml savexml;
    SimpleXmlNode root = savexml.getRoot("ServerListTool");
    root.SetAttribute("FileName", testFileName);
    root.SetAttribute("URL", testUrl);
    root.SetAttribute("Time", static_cast<int>(GetTickCount()));
    root.SetAttribute("UseProxy", static_cast<int>(useProxy));
    root.SetAttribute("ProxyType", static_cast<int>(proxyType));
    root.SetAttribute("ProxyAddress", proxyAddress);
    root.SetAttribute("ProxyPort", proxyPort);
    return savexml.SaveToFile(fileName);
}

void ServersCheckerSettings::copySettings(BasicSettings* dest) {
    dest->ConnectionSettings.UseProxy = useProxy;
    dest->ConnectionSettings.ProxyPort = proxyPort;
    dest->ConnectionSettings.ProxyType = proxyType;
    dest->ConnectionSettings.ServerAddress = proxyAddress;
}

}
