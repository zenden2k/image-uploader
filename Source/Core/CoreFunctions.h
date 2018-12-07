#ifndef IU_CORE_COREFUNCTIONS_H
#define IU_CORE_COREFUNCTIONS_H

#pragma once

#include <memory>
#include "Network/NetworkClient.h"

class INetworkClient;

namespace CoreFunctions {

std::unique_ptr<NetworkClient> createNetworkClient();
void ConfigureProxy(INetworkClient* nm);

}

#endif