#ifndef IU_CORE_UTILS_CORETYPES_H
#define IU_CORE_UTILS_CORETYPES_H

#include <string>
#include "Core/3rdpart/pstdint.h"
#include <memory>

typedef std::string Utf8String;
namespace std_tr = std::tr1;

/*#ifdef _MSC_VER
   typedef __int64 int64_t;
   typedef unsigned __int64 zuint64;
#else
   typedef long long int64_t;
   typedef unsigned long long zuint64;
#endif*/

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&);               \
	void operator=(const TypeName&)

#endif


// The ARRAY_SIZE(arr) macro returns the # of elements in an array arr.
// The expression is a compile-time constant, and therefore can be
// used in defining new arrays, for example.  If you use arraysize on
// a pointer by mistake, you will get a compile-time error.
//
// One caveat is that ARRAY_SIZE() doesn't accept any array of an
// anonymous type or a type defined inside a function.  In these rare
// cases, you have to use the unsafe ARRAYSIZE_UNSAFE() macro below.  This is
// due to a limitation in C++'s template system.  The limitation might
// eventually be removed, but it hasn't happened yet.

// This template function declaration is used in defining ARRAY_SIZE.
// Note that the function doesn't need an implementation, as we only
// use its type.
template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];

// That gcc wants both of these prototypes seems mysterious. VC, for
// its part, can't decide which to use (another mystery). Matching of
// template overloads: the final frontier.
#ifndef _MSC_VER
template <typename T, size_t N>
char (&ArraySizeHelper(const T (&array)[N]))[N];
#endif

#define ARRAY_SIZE(array) (sizeof(ArraySizeHelper(array)))