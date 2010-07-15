/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once
#define GDIPVER 0x0110
#ifndef IU_SHELLEXT
// Change these values to use different versions

#define WINVER		0x0601 // 0x0601 = Windows 7 
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200 

#define _CRT_SECURE_NO_DEPRECATE

#ifdef _ATL_MIN_CRT
#undef _ATL_MIN_CRT
#endif

#undef SPECSTRINGS_H

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

extern CAppModule _Module;

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <map>
#include <GdiPlus.h>
#include <stdio.h>
#include <gdiplus.h>
#include <GdiPlusPixelFormats.h>
#include <atlddx.h>
#include <atlgdi.h>
#include <atlmisc.h>
#include <atlcoll.h>
#include <atltheme.h>
using namespace Gdiplus;
#include "myutils.h"
#include "Common/CmdLine.h"
#include "logwindow.h"
#include "langclass.h"
#include "settings.h"

/*
#include "common.h"



#include "wizarddlg.h"
*/

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif