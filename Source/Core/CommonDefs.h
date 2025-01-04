#ifndef IU_CORE_COMMONDEFS_H
#define IU_CORE_COMMONDEFS_H

#pragma once
//#include "Core/Utils/CoreUtils.h"

#define APPNAME _T("Image Uploader")

// QString <-> std::string (utf-8) conversion macroses
#define Q2U(str) str.toUtf8().data()
#define U2Q(str) QString::fromUtf8(str.c_str())
//#define _(str) str

// CString <-> std::string(utf-8) conversion macroses
#define W2U(str) IuCoreUtils::WstringToUtf8((static_cast<LPCTSTR>(str)))
#define U2W(str) CString(IuCoreUtils::Utf8ToWstring(str).c_str())
#define U2WC(str) (IuCoreUtils::Utf8ToWstring(str).c_str())

#pragma warning(disable:4996)
#pragma warning( disable: 4100 ) // unreferenced formal parameter

constexpr auto IU_VIDEOFILES_EXTENSIONS = "asf;avi;mpeg;mpg;mp2;divx;vob;flv;wmv;mkv;mp4;ts;mov;mpeg2ts;3gp;mpeg1;mpeg2;mpeg4;mv4;rmvb;qt;hdmov;divx;m4v;ogv;m2v;webm";
constexpr auto IU_VIDEOFILES_EXTENSIONS_SEPARATOR = ";";
#endif
