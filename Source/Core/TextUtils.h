
#ifndef IU_CORE_UTILS_TEXTUTILS_H
#define IU_CORE_UTILS_TEXTUTILS_H

#include <cstdio>
#include <string>
#include <vector>
#include "Core/Utils/CoreTypes.h"

namespace IuTextUtils
{
    std::string BbCodeToHtml(const std::string& bbcode);
    bool FileSaveContents(const std::string& fileName, const std::string& contents);
};

#endif
