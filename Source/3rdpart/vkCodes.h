#ifndef VKCODES_H
#define VKCODES_H

#include "atlheaders.h"

PCTSTR GetKeyName(UINT);
BOOL HotkeyToString(UINT, UINT, CString&);
BOOL HotkeyToString(DWORD, CString&);

#endif // VKCODES_H