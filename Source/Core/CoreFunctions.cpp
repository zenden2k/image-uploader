#include "CoreFunctions.h"

#include "Core/Settings.h"
#include "Core/Network/NetworkClient.h"
#ifdef _WIN32
#include "Func/WinUtils.h"
#endif
#include "CommonDefs.h"

namespace CoreFunctions {

void ConfigureProxy(NetworkClient* nm)
{
    if (Settings.ConnectionSettings.UseProxy) {
#ifdef _WIN32
        if (Settings.ConnectionSettings.SystemProxy) {
            CString proxyAddress, ignore;
            if (WinUtils::GetProxyInfo(proxyAddress, ignore)) {
                LOG(ERROR) << proxyAddress;
                // FIXME: this is not working in many cases
                // http://curl.haxx.se/mail/lib-2013-01/att-0327/proxy.cpp
                /*if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig)) {
                    if (ieProxyConfig.fAutoDetect) {
                        fAutoProxy = TRUE;
                    }

                    if (ieProxyConfig.lpszAutoConfigUrl != NULL) {
                        fAutoProxy = TRUE;
                        autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
                    }
                } else {
                    // use autoproxy
                    fAutoProxy = TRUE;
                }

                if (fAutoProxy) {
                    if (autoProxyOptions.lpszAutoConfigUrl != NULL) {
                        autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
                    } else {
                        autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
                        autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
                    }

                    // basic flags you almost always want
                    autoProxyOptions.fAutoLogonIfChallenged = TRUE;

                    // here we reset fAutoProxy in case an auto-proxy isn't actually
                    // configured for this url
                    fAutoProxy = WinHttpGetProxyForUrl(hiOpen, pwszUrl, &autoProxyOptions, &autoProxyInfo);
                }

                if (fAutoProxy) {
                    // set proxy options for libcurl based on autoProxyInfo
                } else {
                    if (ieProxyConfig.lpszProxy != NULL) {
                        // IE has an explicit proxy. set proxy options for libcurl here
                        // based on ieProxyConfig
                        //
                        // note that sometimes IE gives just a single or double colon
                        // for proxy or bypass list, which means "no proxy"
                    } else {
                        // there is no auto proxy and no manually configured proxy
                    }
                }*/
                nm->setProxy(W2U(proxyAddress));
            }
        } else
#endif
        {
            int ProxyTypeList[5] = { CURLPROXY_HTTP, CURLPROXY_SOCKS4, CURLPROXY_SOCKS4A,
                CURLPROXY_SOCKS5, CURLPROXY_SOCKS5_HOSTNAME };
            nm->setProxy(Settings.ConnectionSettings.ServerAddress, Settings.ConnectionSettings.ProxyPort,
                ProxyTypeList[Settings.ConnectionSettings.ProxyType]);
        }

        if (Settings.ConnectionSettings.NeedsAuth) {
            nm->setProxyUserPassword(Settings.ConnectionSettings.ProxyUser,
                Settings.ConnectionSettings.ProxyPassword);
        }
    }
    nm->setUploadBufferSize(Settings.UploadBufferSize);
}

}
