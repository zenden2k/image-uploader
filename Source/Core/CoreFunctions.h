#ifndef IU_CORE_COREFUNCTIONS_H
#define IU_CORE_COREFUNCTIONS_H

#pragma once

#include "Network/NetworkClient.h"

class INetworkClient;

namespace CoreFunctions {

void ConfigureProxy(INetworkClient* nm);

}

#endif