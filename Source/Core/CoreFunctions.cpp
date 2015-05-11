#include "CoreFunctions.h"
#include "Core/Settings.h"
#include "CommonDefs.h"

namespace CoreFunctions {

void ConfigureProxy(NetworkClient* nm)
{
    if (Settings.ConnectionSettings.UseProxy) {
        int ProxyTypeList[5] = { CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A,
            CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME };
        nm->setProxy(W2U(
            (LPCTSTR)Settings.ConnectionSettings.ServerAddress), Settings.ConnectionSettings.ProxyPort,
            ProxyTypeList[Settings.ConnectionSettings.ProxyType]);

        if (Settings.ConnectionSettings.NeedsAuth) {
            nm->setProxyUserPassword(W2U(Settings.ConnectionSettings.ProxyUser),
                Settings.ConnectionSettings.ProxyPassword);
        }
    }
    nm->setUploadBufferSize(Settings.UploadBufferSize);
}

}
