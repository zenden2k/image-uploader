#ifndef IU_CORE_UTILS_STRINGUTILS_H
#define IU_CORE_UTILS_STRINGUTILS_H

#include <cstdio>
#include <string>
#include <vector>
#include "CoreTypes.h"

namespace IuStringUtils
{
	std::string Trim(const std::string& str);
	Utf8String Replace(const Utf8String& text, const Utf8String& s, const Utf8String& d);
	void Split(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount);
};
#endif
