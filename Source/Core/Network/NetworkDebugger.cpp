#include "NetworkDebugger.h"

#include <curl/curl.h>

bool NetworkDebugger::isDebugEnabled() const {
    return !onMessage.empty();
}

void NetworkDebugger::configureClient(INetworkClient* client) {
    //CURL* ch = client->getCurlHandle();
}

int NetworkDebugger::debugCallback(INetworkClient* client, curl_infotype type, char* data, size_t size) {
    onMessage(client, type, data, size);
    return 0;
}
