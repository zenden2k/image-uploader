#ifndef IU_WINHEADERS_H
#define IU_WINHEADERS_H

#pragma once

#define GDIPVER 0x0110
#ifndef IU_SHELLEXT
// Change these values to use different versions

#ifdef WINVER
#undef WINVER
#endif
#define WINVER		0x0601 // 0x0601 = Windows 7 
#define _WIN32_WINNT	0x0601
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200 

#define _CRT_SECURE_NO_DEPRECATE

#ifdef _ATL_MIN_CRT
#undef _ATL_MIN_CRT
#endif

#undef SPECSTRINGS_H
#endif

#include <Windows.h>


#endif