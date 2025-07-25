// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2009 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#pragma once
#include <map>
#include <Windows.h>
#include <Uxtheme.h>
#include "3rdpart/GdiplusH.h"

/*#define STRICT_TYPED_ITEMIDS    // In case you use IDList, you want this on for better type safety.
#include <windows.h>
#include <windowsx.h>           // For WM_COMMAND handling macros
#include <shlobj.h>             // For shell
#include <shlwapi.h>            // QISearch, easy way to implement QI
#include <commctrl.h>
#include <wincodec.h> */          // WIC

typedef HRESULT (WINAPI *FN_GetBufferedPaintBits) (HPAINTBUFFER hBufferedPaint, RGBQUAD **ppbBuffer, int *pcxRow);
typedef HPAINTBUFFER (WINAPI *FN_BeginBufferedPaint) (HDC hdcTarget, const RECT *prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc);
typedef HRESULT (WINAPI *FN_EndBufferedPaint) (HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);


/**
 * \ingroup utils
 * provides helper functions for converting icons to bitmaps
 */
class IconBitmapUtils
{
public:
    IconBitmapUtils(void);
    ~IconBitmapUtils(void);

    HBITMAP IconToBitmap(HINSTANCE hInst, UINT uIcon);
    HBITMAP HIconToBitmapPARGB32(HICON hIcon, int dpi);
    HBITMAP IconToBitmapPARGB32(HINSTANCE hInst, UINT uIcon, int dpi);
    HRESULT Create32BitHBITMAP(HDC hdc, const SIZE *psize, __deref_opt_out void **ppvBits, __out HBITMAP* phBmp);
    HRESULT ConvertBufferToPARGB32(HPAINTBUFFER hPaintBuffer, HDC hdc, HICON hicon, SIZE& sizIcon);
    bool HasAlpha(__in Gdiplus::ARGB *pargb, SIZE& sizImage, int cxRow);
    HRESULT ConvertToPARGB32(HDC hdc, __inout Gdiplus::ARGB *pargb, HBITMAP hbmp, SIZE& sizImage, int cxRow);


private:
    HMODULE hUxTheme;
    std::map<UINT, HBITMAP>        bitmaps;

    FN_GetBufferedPaintBits pfnGetBufferedPaintBits;
    FN_BeginBufferedPaint pfnBeginBufferedPaint;
    FN_EndBufferedPaint pfnEndBufferedPaint;
};
