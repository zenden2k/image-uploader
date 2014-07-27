
#ifndef IU_CORE_UTILS_TEXTUTILS_H
#define IU_CORE_UTILS_TEXTUTILS_H

#include <cstdio>
#include <string>
#include <vector>
#include <Core/Utils/CoreTypes.h>

namespace IuTextUtils
{
	Utf8String BbCodeToHtml(const Utf8String& bbcode);
	bool FileSaveContents(const Utf8String& fileName, const Utf8String& contents);
};

#endif
