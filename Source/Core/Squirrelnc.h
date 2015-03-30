#ifndef CORE_SQUIRREL_INC_H
#define CORE_SQUIRREL_INC_H

#ifdef UNICODE
#undef UNICODE // We do not want to compile sqplus with unicode support
#undef _UNICODE
#include <sqplus.h>
#define UNICODE
#define _UNICODE
#else
#include <sqplus.h>
#endif

#endif
