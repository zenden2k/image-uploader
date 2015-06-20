#ifndef IU_CORE_UTILS_SYSTEMUTILS_WIN_H
#define IU_CORE_UTILS_SYSTEMUTILS_WIN_H

#include <string>
#include <limits.h>

namespace IuCoreUtils
{

bool isOs64Bit() {
    if (sizeof(void *) * CHAR_BIT == 64) {
        return true;
    }
    else {
        return false;
    }
}

std::string getOsName() {
    // TODO:
    return "Unix";
}

std::string getOsVersion() {
    // TODO: 
    std::string res;
    
    return res;
}

}

#endif