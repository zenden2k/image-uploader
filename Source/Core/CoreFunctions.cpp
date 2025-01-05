#include "CoreFunctions.h"

#include "Core/Network/NetworkClient.h"
#include "Settings/BasicSettings.h"
#include "ServiceLocator.h"
#ifdef _WIN32
#include "DefaultProxyProvider.h"
#endif
#include "Core/Network/NetworkDebugger.h"

namespace CoreFunctions {

void ConfigureProxy(INetworkClient* nm)
{
    auto debugger = ServiceLocator::instance()->networkDebugger();
    nm->setDebugger(debugger);
    BasicSettings& Settings = *ServiceLocator::instance()->basicSettings();
#ifdef _WIN32
    if (Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kSystemProxy) {
        nm->setProxyProvider(std::make_shared<DefaultProxyProvider>());
    } else
#endif
    if (Settings.ConnectionSettings.UseProxy == ConnectionSettingsStruct::kUserProxy) {
        int ProxyTypeList[6] = { CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A, CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME, CURLPROXY_HTTPS };
        if (Settings.ConnectionSettings.ProxyType >= 0 && Settings.ConnectionSettings.ProxyType < ARRAY_SIZE(ProxyTypeList) ) {
            nm->setProxy(Settings.ConnectionSettings.ServerAddress, Settings.ConnectionSettings.ProxyPort,
                ProxyTypeList[Settings.ConnectionSettings.ProxyType]);

            if (Settings.ConnectionSettings.NeedsAuth) {
                nm->setProxyUserPassword(Settings.ConnectionSettings.ProxyUser,
                    Settings.ConnectionSettings.ProxyPassword);
            }
        }
    }
    nm->setUploadBufferSize(Settings.UploadBufferSize);
    nm->setMaxUploadSpeed(Settings.MaxUploadSpeed*1024);
    /* curl_easy_setopt(nm->getCurlHandle(), CURLOPT_STDERR, fopen("d:/curl_log.txt", "w"));
    curl_easy_setopt(nm->getCurlHandle(), CURLOPT_VERBOSE, 1L);*/
}

}
