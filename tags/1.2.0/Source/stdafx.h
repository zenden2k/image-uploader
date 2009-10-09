// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions

// Закомментируйте строку для "интернациональной версии"
#define MD_VERSION

#define WINVER		0x0700
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200 

/*#define __AFX_H__
struct __POSITION { int unused; };*/

#define _CRT_SECURE_NO_DEPRECATE

#ifdef _ATL_MIN_CRT
#undef _ATL_MIN_CRT
#endif

#undef SPECSTRINGS_H

#include <atlbase.h>
#include <atlapp.h>
//#define __allowed(p)
//#include <specstrings.h>
#define APPNAME _T("Image Uploader")
#define VIDEO_DIALOG_FORMATS _T("Video files (avi, mpg, vob, wmv, flv, etc)\0*.avi;*.mpeg;*.mpg;*.mp2;*.divx;*.vob;*.flv;*.wmv;*.asf;*.mkv;*.mp4;*.ts;*.mov;*.mpeg2ts;*.3gp;\0All files\0*.*\0\0")
#define VIDEO_FORMATS _T("avi\0mpg\0mpeg\0vob\0divx\0flv\0wmv\0asf\0mkv\0mov\0ts\0mp2\0mp4\0")_T("3gp\0rm\0mpeg2ts\0\0")
#define IMAGE_DIALOG_FORMATS _T("Image files (JPEG, GIF, PNG, etc)\0*.jpg;*.gif;*.png;*.bmp;*.tiff\0All files\0*.*\0\0")
extern CAppModule _Module;

#ifdef MD_VERSION
#define MD_VER 1
#else 
#define MD_VER 0
#endif

#define NUMBER_OF_SERVERS 5+3*MD_VER
//extern TCHAR UploadServers[NUMBER_OF_SERVERS][50];
#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <map>
#include <GdiPlus.h>
#include <objbase.h>
#include <streams.h>
#include <stdio.h>
#include <atlbase.h>

//#include <qedit.h>
#include <gdiplus.h>
#include <GdiPlusPixelFormats.h>

#include <atlddx.h>
#include <atlgdi.h>
#include <atlmisc.h>
#include <atlcoll.h>
using namespace Gdiplus;

#include "myutils.h"
#include "Common/CmdLine.h"

#include "common.h"
#include "langclass.h"
#include "logwindow.h"

#include "wizarddlg.h"
#include "settings.h"

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
