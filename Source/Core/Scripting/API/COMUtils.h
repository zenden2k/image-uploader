#ifndef IU_CORE_SCRIPTAPI_COMUTILS_H
#define IU_CORE_SCRIPTAPI_COMUTILS_H
#include "atlheaders.h"
#include <string>
#pragma once
namespace ScriptAPI {
    std::string ComVariantToString(const CComVariant& variant);
}

#endif