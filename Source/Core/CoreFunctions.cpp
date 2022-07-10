#include "CoreFunctions.h"

#include "Core/Network/NetworkClient.h"
#include "Settings/BasicSettings.h"
#include "ServiceLocator.h"
#ifdef _WIN32
#include "DefaultProxyProvider.h"
#endif

namespace CoreFunctions {

void ConfigureProxy(INetworkClient* nm)
{
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
#ifdef _WIN32
    if (Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kSystemProxy) {
        nm->setProxyProvider(std::make_shared<DefaultProxyProvider>());
    } else
#endif
    if (Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kUserProxy) {
        int ProxyTypeList[5] = { CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME };
        nm->setProxy(Settings.ConnectionSettings.ServerAddress, Settings.ConnectionSettings.ProxyPort,
                        ProxyTypeList[Settings.ConnectionSettings.ProxyType]);

        if (Settings.ConnectionSettings.NeedsAuth) {
            nm->setProxyUserPassword(Settings.ConnectionSettings.ProxyUser,
                Settings.ConnectionSettings.ProxyPassword);
        }
    }
    nm->setUploadBufferSize(Settings.UploadBufferSize);
    nm->setMaxUploadSpeed(Settings.MaxUploadSpeed*1024);
}

}
