#ifndef IU_ATLHEADERS_H
#define IU_ATLHEADERS_H

#pragma once
#ifndef _UNICODE
#define UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN 
//#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <ShellAPI.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlddx.h>
#include <atlgdi.h>
#include <atlmisc.h>
#include <atlcoll.h>
#include <atltheme.h>
#include <atlcrack.h>
#include "atlctrlx.h"
#include "Func/LangClass.h"

typedef CAtlArray<CString> CStringList;
#ifndef IU_SHELLEXT
extern CAppModule _Module;
#else 
extern HINSTANCE hDllInstance;
#endif
#endif