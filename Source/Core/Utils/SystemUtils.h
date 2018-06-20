#ifndef IU_CORE_SYSTEMUTILS_H
#define IU_CORE_SYSTEMUTILS_H

#pragma once

#include <string>

namespace IuCoreUtils
{
    std::string GetOsName();
    std::string GetOsVersion();
    std::string GetCpuFeatures();
    bool IsOs64Bit();
}

#endif
