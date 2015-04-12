#ifndef CORE_SQUIRREL_INC_H
#define CORE_SQUIRREL_INC_H

#ifdef UNICODE
#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#endif
#undef UNICODE // We do not want to compile sqplus with unicode support
#undef _UNICODE
#include <squirrel.h>
#include <sqrat.h>
#include <sqrat/sqratVM.h>
#define UNICODE
#define _UNICODE
#else
#include <sqplus.h>
#endif

#ifndef _SQ64
#define SQINT_TO_JSON_VALUE(a) a
#else 
#define SQINT_TO_JSON_VALUE(a) ((Json::Int64)a)
#endif


#endif
