#ifndef IU_CORE_UTILS_CORETYPES_H
#define IU_CORE_UTILS_CORETYPES_H

#include <cstdio>
#include <string>

typedef std::string Utf8String;

#ifdef _MSC_VER
   typedef __int64 zint64;
   typedef unsigned __int64 zuint64;

#else
   typedef long long zint64;
   typedef unsigned long long zuint64;
#endif

	// A macro to disallow the copy constructor and operator= functions
	// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&);               \
	void operator=(const TypeName&)

#endif
