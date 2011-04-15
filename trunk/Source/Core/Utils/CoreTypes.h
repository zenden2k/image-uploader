#ifndef IU_CORE_UTILS_CORETYPES_H
#define IU_CORE_UTILS_CORETYPES_H

#include <cstdio>
#include <string>
typedef std::string Utf8String;
#ifdef _MSC_VER
	#define zint64 __int64
#else
	#define zint64  long long
#endif

#endif