#ifndef IU_CORE_COREFUNCTIONS_H
#define IU_CORE_COREFUNCTIONS_H

#pragma once

class NetworkClient;

namespace CoreFunctions {

std::unique_ptr<NetworkClient> createNetworkClient();
void ConfigureProxy(NetworkClient* nm);

}

#endif