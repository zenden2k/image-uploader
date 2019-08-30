#include "ServersCheckerSettings.h"

#include "Func/WinUtils.h"
#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Settings/BasicSettings.h"

namespace ServersListTool {
   
ServersCheckerSettings::ServersCheckerSettings() {
    rootName_ = "ServersChecker";
    testFileName = W2U(WinUtils::GetAppFolder()) + "testfile.jpg";
    testUrl = "https://github.com/zenden2k/image-uploader/issues";

    BindToManager();
}

void ServersCheckerSettings::BindToManager() {
    BasicSettings::BindToManager();
    SettingsNode& serversChecker = mgr_["Basic"];
    serversChecker.n_bind(testFileName);
    serversChecker.n_bind(testUrl);

    SettingsNode& proxy = serversChecker["Proxy"];
    proxy["@UseProxy"].bind(ConnectionSettings.UseProxy);
    proxy["@NeedsAuth"].bind(ConnectionSettings.NeedsAuth);
    proxy.nm_bind(ConnectionSettings, ServerAddress);
    proxy.nm_bind(ConnectionSettings, ProxyPort);
    proxy.nm_bind(ConnectionSettings, ProxyType);
    //proxy.nm_bind(ConnectionSettings, ProxyUser);
    //proxy.nm_bind(ConnectionSettings, ProxyPassword);;
}

bool ServersCheckerSettings::LoadSettings(const std::string& szDir, const std::string& fileName) {
    return BasicSettings::LoadSettings(szDir, fileName, false);
}

bool ServersCheckerSettings::PostLoadSettings(SimpleXml &xml) {
    SimpleXmlNode root = xml.getRoot("ServerListTool");
    std::string name = root.Attribute("FileName");
    if (!name.empty()) {
        testFileName = name;
    }
    std::string url = root.Attribute("URL");
    if (!url.empty()) {
        testUrl = url;
    }
    return true;
}

}
