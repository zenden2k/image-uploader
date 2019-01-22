#ifndef IU_CORE_COMMONDEFS_H
#define IU_CORE_COMMONDEFS_H

#pragma once
#include "Core/Utils/CoreUtils.h"

#define APPNAME _T("Image Uploader")

// QString <-> std::string (utf-8) conversion macroses
#define Q2U(str) str.toUtf8().data()
#define U2Q(str) QString::fromUtf8(str.c_str())
#define _(str) str

// CString <-> std::string(utf-8) conversion macroses
#define W2U(str) IuCoreUtils::WstringToUtf8(((LPCTSTR)(str)))
#define U2W(str) CString(IuCoreUtils::Utf8ToWstring(str).c_str())

#pragma warning(disable:4996)
#pragma warning( disable: 4100 ) // unreferenced formal parameter

#endif
