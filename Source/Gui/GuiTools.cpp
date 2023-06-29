/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "Gui/GuiTools.h"

#include <cmath>
#include <cstdarg>

#include <ShObjIdl.h>

#include "Func/WinUtils.h"
#include "Core/ServiceLocator.h"
#include "Core/i18n/Translator.h"

namespace GuiTools
{
int AddComboBoxItem(HWND hDlg, int itemId, LPCTSTR item) {
    return ::SendDlgItemMessage(hDlg, itemId, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item));
}

bool AddComboBoxItems(HWND hDlg, int itemId, int itemCount, ...) {
    bool result = true;
    std::va_list args;
    va_start(args, itemCount);
    for (int i = 0; i < itemCount; ++i) {
        if (AddComboBoxItem(hDlg, itemId, va_arg(args, LPCTSTR)) < 0)
            result = false;
    }
    va_end(args);
    return result;
}

bool IsChecked(HWND dlg, int id) {
    return ::SendDlgItemMessage(dlg, id,BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void GetCheck(HWND dlg, int id, bool& check) {
    check = ::SendDlgItemMessage(dlg, id,BM_GETCHECK, 0, 0) == BST_CHECKED;
}

bool GetCheck(HWND dlg, int id) {
    return ::SendDlgItemMessage(dlg, id,BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void SetCheck(HWND dlg, int id, bool check) {
    ::SendDlgItemMessage(dlg, id,BM_SETCHECK, check, 0);
}

HFONT MakeLabelBold(HWND Label) {
    HFONT Font = reinterpret_cast<HFONT>(SendMessage(Label, WM_GETFONT, 0, 0));

    if (!Font) return nullptr;

    LOGFONT alf;

    if (!(::GetObject(Font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT))) return nullptr;

    alf.lfWeight = FW_BOLD;

    HFONT NewFont = CreateFontIndirect(&alf);
    SendMessage(Label,WM_SETFONT, reinterpret_cast<WPARAM>(NewFont), MAKELPARAM(false, 0));
    CWindowDC dc(0);
    alf.lfHeight = -MulDiv(13, GetDeviceCaps(dc, LOGPIXELSY), 72);
    return NewFont;
}

HFONT MakeLabelItalic(HWND Label) {
    HFONT Font = reinterpret_cast<HFONT>(SendMessage(Label, WM_GETFONT, 0, 0));

    if (!Font) return nullptr;

    LOGFONT alf;

    if (!(::GetObject(Font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT))) return nullptr;

    alf.lfItalic = 1;

    HFONT NewFont = CreateFontIndirect(&alf);
    SendMessage(Label,WM_SETFONT, reinterpret_cast<WPARAM>(NewFont),MAKELPARAM(false, 0));
    return NewFont;
}

void EnableNextN(HWND Control, int n, bool Enable) {
    for (int i = 0; i < n; i++) {
        Control = GetNextWindow(Control, GW_HWNDNEXT);
        EnableWindow(Control, Enable);
    }
}

bool InsertMenu(HMENU hMenu, int pos, UINT id, LPCTSTR szTitle, HBITMAP bm){
    MENUITEMINFO MenuItem;

    MenuItem.cbSize = sizeof(MenuItem);
    if(szTitle)
        MenuItem.fType = MFT_STRING;
    else MenuItem.fType = MFT_SEPARATOR;
    MenuItem.fMask = MIIM_TYPE | MIIM_ID | MIIM_DATA;
    if(bm)
        MenuItem.fMask |= MIIM_CHECKMARKS;
    MenuItem.wID = id;
    MenuItem.hbmpChecked = bm;
    MenuItem.hbmpUnchecked = bm;
    MenuItem.dwTypeData = const_cast<LPWSTR>(szTitle);
    return InsertMenuItem(hMenu, pos, TRUE, &MenuItem)!=0;
}

void FillRectGradient(HDC hdc, const RECT& FillRect, COLORREF start, COLORREF finish, bool Horizontal) {
        RECT rectFill;          
        float fStep;            //The size of each band in pixels
        HBRUSH hBrush;
        int i;  // Loop index

        float r, g, b;
        int n = 256;
        //FillRect.bottom--;
        COLORREF c;

        if(!Horizontal)
            fStep = (FillRect.bottom - FillRect.top) / 256.0f;
        else 
            fStep = (FillRect.right - FillRect.left) / 256.0f;

        if( fStep < 1)
        {
            fStep = 1;
            if(!Horizontal)
                n = FillRect.bottom - FillRect.top;
            else 
                n = (FillRect.right - FillRect.left);
        }

        r = static_cast<float>(GetRValue(finish)-GetRValue(start))/(n-1);

        g = static_cast<float>(GetGValue(finish)-GetGValue(start))/(n-1);

        b = static_cast<float>(GetBValue(finish)-GetBValue(start))/(n-1);

        //Begin paint
        for (i = 0; i < n; i++) 
        {
            // Horizontal or vertical gradient
            if(!Horizontal)
                SetRect(&rectFill, FillRect.left, int((i * fStep)+FillRect.top),
                FillRect.right+1, int(FillRect.top+(i+1) * fStep)); 
            else 
                SetRect(&rectFill, static_cast<int>(FillRect.left+(i * fStep)), FillRect.top,
                int(FillRect.left+((i+1) * fStep)), FillRect.bottom+1); 
            if(i == n-1)
                c = finish;
            else
                c = RGB((int)GetRValue(start)+(r*i/**zR*/),(int)GetGValue(start)+(g*i/*zG*/),(int)GetBValue(start)+(b*i/**zB*/));

            hBrush=CreateSolidBrush(c);

            ::FillRect(hdc, &rectFill, hBrush);

            DeleteObject(hBrush);
        }
}
/*
* Fails on armv8
bool SelectDialogFilter(LPTSTR szBuffer, int nMaxSize, int nCount, LPCTSTR szName, LPCTSTR szFilter,...) {
    *szBuffer = 0;
    LPCTSTR *pszName, *pszFilter;
    pszName = &szName;
    pszFilter = &szFilter;

    for (int i = 0; i < nCount; i++) {
        int nLen = lstrlen(*pszName);
        lstrcpy(szBuffer, *pszName);
        szBuffer[nLen] = 0;
        szBuffer += nLen + 1;

        nLen = lstrlen(*pszFilter);
        lstrcpy(szBuffer, *pszFilter);
        szBuffer[nLen] = 0;
        szBuffer += nLen + 1;
        pszName += 2;
        pszFilter += 2;
    }
    *szBuffer = 0;
    return true;
}
*/
// Converts pixels to Win32 dialog units
int dlgX(int WidthInPixels) {
    LONG units = GetDialogBaseUnits();
    short baseunitX = LOWORD(units);
    return WidthInPixels * baseunitX / 4;
}

// Converts pixels to Win32 dialog units
int dlgY(int HeightInPixels) {
    LONG units = GetDialogBaseUnits();
    short baseunitY = HIWORD(units);
    return HeightInPixels * baseunitY / 8;
}

CString GetWindowText(HWND wnd) {
    int len = GetWindowTextLength(wnd);
    if (!len) {
        return {};
    }
    CString buf;
    GetWindowText(wnd, buf.GetBuffer(len + 1), len + 1);
    buf.ReleaseBuffer(-1);
    return buf;
}

CString GetDlgItemText(HWND dialog, int controlId) {
    return GetWindowText(::GetDlgItem(dialog, controlId ) );
}

/* MakeFontBold
    MakeFontUnderLine

    -----------------------
    These functions create bold/underlined fonts based on given font
*/
HFONT MakeFontBold(HFONT font) {
    if ( !font ) {
        return 0;
    }

    LOGFONT alf;

    bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

    if(!ok) return 0;

    alf.lfWeight = FW_BOLD;

    HFONT NewFont = CreateFontIndirect(&alf);
    return NewFont;
}

HFONT MakeFontUnderLine(HFONT font)
{
    if(!font) return 0;

    LOGFONT alf;

    bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);

    if(!ok) return 0;

    alf.lfUnderline = 1;
    HFONT NewFont = CreateFontIndirect(&alf);

    return NewFont;
}

HFONT MakeFontSmaller(HFONT font) {
    if ( !font ) {
        return 0;
    }

    LOGFONT alf;
    bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);
    if ( !ok ) {
        return 0;
    }

    alf.lfHeight = GetFontSize( 12 );
    HFONT NewFont = CreateFontIndirect(&alf);
    return NewFont;
}

HFONT MakeFontBigger(HFONT font) {
    if ( !font ) {
        return 0;
    }

    LOGFONT alf;
    bool ok = ::GetObject(font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT);
    if ( !ok ) {
        return 0;
    }

    alf.lfHeight += GetFontSize( 5 );
    alf.lfWeight = FW_BOLD;
    HFONT NewFont = CreateFontIndirect(&alf);
    return NewFont;
}
int GetFontSize(int nFontHeight) {
    HDC dc = ::GetDC(nullptr);
    int res =  - MulDiv( nFontHeight, 72, GetDeviceCaps(dc, LOGPIXELSY));
    ReleaseDC(nullptr, dc);
    return res;
}

int GetFontHeight(int nFontSize) {
    HDC dc = ::GetDC(nullptr);
    int res =  - MulDiv(nFontSize, GetDeviceCaps(dc, LOGPIXELSY), 72);
    ReleaseDC(nullptr, dc);
    return res;
}

HFONT GetSystemDialogFont()
{
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);
#if (WINVER >= 0x0600)
    if ( !WinUtils::IsVistaOrLater() ) {
        ncm.cbSize = sizeof(NONCLIENTMETRICS) - sizeof(int);
    }
#endif

    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0)) {
        return CreateFontIndirect(&ncm.lfMessageFont);
    }
    return 0;
}

int ScreenBPP(){
    int iRet = 0;
    HDC hdc = GetDC(NULL);
    if (hdc != NULL) {
        iRet = GetDeviceCaps(hdc, BITSPIXEL);
        ReleaseDC(NULL, hdc);
    }
    return iRet;
}

BOOL Is32BPP(){
    return ScreenBPP() >= 32;
}

RECT GetDialogItemRect(HWND dialog, int itemId) {
    HWND control = ::GetDlgItem( dialog, itemId );
    RECT controlRect={0,0,0,0};
    GetWindowRect(control, &controlRect );
    MapWindowPoints(0 /*means desktop*/, dialog, reinterpret_cast<LPPOINT>(&controlRect), 2);
    return controlRect;
}

void ShowDialogItem(HWND dlg, int itemId, bool show) {
    ::ShowWindow(GetDlgItem(dlg, itemId), show? SW_SHOW : SW_HIDE);
}

RECT AutoSizeStaticControl(HWND control) {
    CString text = GuiTools::GetWindowText(control);
    CClientDC dc(control);
    HFONT font = reinterpret_cast<HFONT>(SendMessage(control, WM_GETFONT, 0, 0));

    SIZE textSize;
    HGDIOBJ oldFont = SelectObject(dc, font);
    GetTextExtentPoint32(dc, text, text.GetLength(), &textSize);
    SetWindowPos(control, nullptr, 0, 0, textSize.cx, textSize.cy, SWP_NOMOVE|SWP_NOZORDER);
    RECT controlRect;
    GetWindowRect(control, &controlRect);
    MapWindowPoints(0 /*means desktop*/, GetParent(control), reinterpret_cast<LPPOINT>(&controlRect), 2);
    SelectObject(dc, oldFont);
    ::InvalidateRect(control, 0, FALSE);
    return controlRect;
}

void EnableDialogItem(HWND dlg, int itemId, bool enable) {
    ::EnableWindow( ::GetDlgItem( dlg, itemId ), enable );
}

IconInfo GetIconInfo(HICON hIcon)
{
    IconInfo myinfo;
    ZeroMemory(&myinfo, sizeof(myinfo));

    ICONINFO info;
    ZeroMemory(&info, sizeof(info));

    BOOL bRes = FALSE;

    bRes = GetIconInfo(hIcon, &info);
    if(!bRes)
        return myinfo;

    BITMAP bmp;
    ZeroMemory(&bmp, sizeof(bmp));

    if(info.hbmColor)
    {
        const int nWrittenBytes = GetObject(info.hbmColor, sizeof(bmp), &bmp);
        if(nWrittenBytes > 0)
        {
            myinfo.nWidth = bmp.bmWidth;
            myinfo.nHeight = bmp.bmHeight;
            myinfo.nBitsPerPixel = bmp.bmBitsPixel;
        }
    }
    else if(info.hbmMask)
    {
        // Icon has no color plane, image data stored in mask
        const int nWrittenBytes = GetObject(info.hbmMask, sizeof(bmp), &bmp);
        if(nWrittenBytes > 0)
        {
            myinfo.nWidth = bmp.bmWidth;
            myinfo.nHeight = bmp.bmHeight / 2;
            myinfo.nBitsPerPixel = 1;
        }
    }

    if(info.hbmColor)
        DeleteObject(info.hbmColor);
    if(info.hbmMask)
        DeleteObject(info.hbmMask);

    return myinfo;
}

int GetWindowLeft(HWND Wnd)
{
    RECT WindowRect = {0,0,0,0};

    GetWindowRect(Wnd,&WindowRect);
    HWND Parent = GetParent(Wnd);
    ScreenToClient(Parent, reinterpret_cast<LPPOINT>(&WindowRect));
    return WindowRect.left;
}

std::vector<RECT> monitorsRects;
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    if (lprcMonitor)
    {
        monitorsRects.push_back(*lprcMonitor);
    }
    return TRUE;
}

bool GetScreenBounds(RECT& rect)
{
    monitorsRects.clear();
    EnumDisplayMonitors(0, 0, MonitorEnumProc, 0);
    CRect result;
    for (size_t i = 0; i < monitorsRects.size(); i++)
    {
        CRect Bounds = monitorsRects[i];
        result.UnionRect(result, Bounds);
    }
    rect = result;
    return true;
}

HRGN CloneRegion(HRGN source)
{
    HRGN resultRgn = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(resultRgn, source, resultRgn, RGN_OR);
    return resultRgn;
}

HWND CreateToolTipForWindow(HWND hwnd, const CString& text) {
    HWND hwndTT = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, 
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
        hwnd, NULL, _Module.GetModuleInstance(),NULL);

    ::SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, 
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    RECT clientRect;
    ::GetClientRect(hwnd, &clientRect);

    TOOLINFO ti = {};
    ti.cbSize   = sizeof(TOOLINFO);
    ti.uFlags   = TTF_SUBCLASS;
    ti.hwnd     = hwnd;
    ti.hinst    = _Module.GetModuleInstance();
    auto textBuffer = std::make_unique<TCHAR[]>(text.GetLength() + 1);
    lstrcpy(textBuffer.get(), text);
    ti.lpszText = textBuffer.get();
    ti.rect  = clientRect;

    // Associate the tooltip with the "tool" window.
    SendMessage(hwndTT, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));

    return hwndTT;
}

void AddToolTip(HWND hwndTT, HWND hwnd, const CString& text) {
    // Set up "tool" information. In this case, the "tool" is the entire parent window.
    RECT clientRect;
    ::GetClientRect(hwnd, &clientRect);
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = hwnd;
    ti.hinst = _Module.GetModuleInstance();
    auto textBuffer = std::make_unique<TCHAR[]>(text.GetLength() + 1);
    lstrcpy(textBuffer.get(), text);
    ti.lpszText = textBuffer.get();
    ti.rect = clientRect;

    // Associate the tooltip with the "tool" window.
    SendMessage(hwndTT, TTM_ADDTOOL, 0, reinterpret_cast<LPARAM>(&ti));
}

CHARFORMAT LogFontToCharFormat(const LOGFONT & lf)
{
    CHARFORMAT cf;
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask =  CFM_FACE | CFM_SIZE | CFM_CHARSET 
        | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_OFFSET;
    cf.dwEffects = 0;

    if (lf.lfWeight >= FW_BOLD)
    {
        cf.dwEffects |= CFE_BOLD;
    }

    if (lf.lfUnderline)
    {
        cf.dwEffects |= CFE_UNDERLINE;
    }

    if (lf.lfItalic)
    {
        cf.dwEffects |= CFE_ITALIC;
    }

    if (lf.lfStrikeOut)
    {
        cf.dwEffects |= CFE_STRIKEOUT;
    }

    //temporary create DC
    CDC dc;
    dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
    cf.yHeight = 20*long( 0.5 + fabs(double(72*lf.lfHeight)/dc.GetDeviceCaps(LOGPIXELSY)));
    dc.DeleteDC();

    cf.yOffset = 0;
    cf.bCharSet = lf.lfCharSet;
    cf.bPitchAndFamily = lf.lfPitchAndFamily;
    _tcscpy_s(cf.szFaceName, LF_FACESIZE, lf.lfFaceName);
    return cf;
}

LOGFONT CharFormatToLogFont(const CHARFORMAT & cf)
{
    LOGFONT lf;
    lf.lfCharSet = cf.bCharSet;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH;

    if ( (cf.dwEffects & CFE_BOLD) == CFE_BOLD) {
        lf.lfWeight = FW_BOLD;
    }

    CDC dc;
    dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
    lf.lfHeight = -MulDiv(cf.yHeight/20, dc.GetDeviceCaps(LOGPIXELSY), 72);
    dc.DeleteDC();

    lf.lfUnderline = ( (cf.dwEffects & CFE_UNDERLINE) == CFE_UNDERLINE);
    lf.lfStrikeOut = ( (cf.dwEffects & CFE_STRIKEOUT) == CFE_STRIKEOUT);
    lf.lfItalic = ( (cf.dwEffects & CFE_ITALIC) == CFE_ITALIC);

    lf.lfWidth = 0;
    _tcscpy_s(lf.lfFaceName, LF_FACESIZE, cf.szFaceName);

    return lf;
}

HICON LoadSmallIcon(int resourceId) {
    int iconWidth =  ::GetSystemMetrics(SM_CXSMICON);
    int iconHeight =  ::GetSystemMetrics(SM_CYSMICON);
    if ( iconWidth > 16 ) {
        iconWidth = 48;
    } 
    
    if ( iconHeight > 16 ) {
        iconHeight = 48;
    } 

    return static_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(resourceId), 
        IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR));
}

HICON LoadBigIcon(int resourceId) {
    int iconWidth = ::GetSystemMetrics(SM_CXICON);
    int iconHeight = ::GetSystemMetrics(SM_CYICON);


    HICON result = nullptr;
    LoadIconWithScaleDown(_Module.GetResourceInstance(), MAKEINTRESOURCE(resourceId), iconWidth, iconHeight, &result);
    return result;

    //return static_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(resourceId), IMAGE_ICON, iconWidth, iconHeight, LR_DEFAULTCOLOR));
}

void RemoveWindowStyleEx(HWND hWnd, DWORD styleEx) {
    LONG oldStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
    ::SetWindowLong(hWnd, GWL_EXSTYLE, oldStyle & ~styleEx);
}

int LocalizedMessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) {
    if (ServiceLocator::instance()->translator()->isRTL()) {
        uType |= MB_RTLREADING;
    }
    return MessageBox(hWnd, lpText, lpCaption, uType);
}

BOOL SetClientRect(HWND hWnd, int x, int y)
{
    RECT rect = {0,0,x,y}, rect2;
    AdjustWindowRectEx(&rect, GetWindowLong(hWnd,GWL_STYLE), (BOOL)GetMenu(hWnd), GetWindowLong(hWnd, GWL_EXSTYLE));
    GetWindowRect(hWnd, &rect2);
    return MoveWindow(hWnd, rect2.left, rect2.top, rect.right-rect.left,rect.bottom-rect.top, TRUE);
}
}
