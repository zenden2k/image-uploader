#pragma once

#include <curl/curl.h>
#include <boost/signals2.hpp>

#include "INetworkClient.h"

class NetworkDebugger: public INetworkClient::Debugger {
public:
    boost::signals2::signal<void(INetworkClient*, curl_infotype, char*, size_t)> onMessage;
    
//private:
    void configureClient(INetworkClient* client) override;
    int debugCallback(INetworkClient* client, curl_infotype type, char* data, size_t size) override;
    bool isDebugEnabled() const override;
    friend class INetworkClient;
};
