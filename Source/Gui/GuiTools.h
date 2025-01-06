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
#ifndef IU_GUITOOLS_H
#define IU_GUITOOLS_H
#pragma once

#include "atlheaders.h"
#include <windows.h>

namespace GuiTools
{
    int AddComboBoxItem(HWND hDlg, int itemId, LPCTSTR item);
    bool AddComboBoxItems(HWND hDlg, int itemId, int itemCount, ...);
    void GetCheck(HWND dlg, int id, bool& check);
    bool GetCheck(HWND dlg, int id);
    void SetCheck(HWND dlg, int id, bool check);
    [[nodiscard]] HFONT MakeLabelBold(HWND Label);
    void EnableNextN(HWND Control, int n, bool Enable);
    bool InsertMenu(HMENU hMenu, int pos, UINT id, LPCTSTR szTitle, HBITMAP bm = 0);
    void FillRectGradient(HDC hdc, const RECT& FillRect, COLORREF start, COLORREF finish, bool Horizontal);
    RECT GetDialogItemRect(HWND dialog, int itemId);
    void ShowDialogItem(HWND dlg, int itemId, bool show);
    void EnableDialogItem(HWND dlg, int itemId, bool enable);
    void RemoveWindowStyleEx(HWND hWnd, DWORD styleEx);
    
    // Converts pixels to Win32 dialog units
    int dlgX(int WidthInPixels);
    int dlgY(int HeightInPixels);

    CString GetWindowText(HWND wnd);
    CString GetDlgItemText(HWND dialog, int controlId);

    int GetWindowLeft(HWND Wnd);

    HFONT MakeFontUnderLine(HFONT font);
    HFONT MakeFontBold(HFONT font);
    HFONT MakeFontSmaller(HFONT font);
    [[nodiscard]] HFONT MakeLabelItalic(HWND Label);

    int GetFontSize(int nFontHeight);
    int GetFontHeight(int nFontSize);
    HFONT GetSystemDialogFont();

    int ScreenBPP();
    BOOL Is32BPP();

    bool IsChecked(HWND dlg, int id);

    RECT AutoSizeStaticControl(HWND control);

    struct IconInfo
    {
        int     nWidth;
        int     nHeight;
        int     nBitsPerPixel;
    };

    IconInfo GetIconInfo(HICON hIcon);

    bool GetScreenBounds(RECT& rect);
    HRGN CloneRegion(HRGN source);
    HFONT MakeFontBigger(HFONT font);
    HWND CreateToolTipForWindow(HWND hwnd, const CString& text);
    void AddToolTip(HWND hwndTT, HWND hwnd, const CString& text);
    CHARFORMAT LogFontToCharFormat(const LOGFONT & lf);
    LOGFONT CharFormatToLogFont(const CHARFORMAT & cf);
    HICON LoadSmallIcon(int resourceId);
    HICON LoadBigIcon(int resourceId);
    int LocalizedMessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption = _T(""), UINT uType = MB_OK);
    BOOL SetClientRect(HWND hWnd, int x, int y);
    BOOL IsWindowCloaked(HWND hwnd);
    BOOL IsWindowVisibleOnScreen(HWND hwnd);

    void SetWindowPointer(HWND hwnd, void* pthis);
    // We need this function to check in lambda if CWindow* 'this' is still alive.
    // The function checks if the window exists and belongs to the current class.
    bool CheckWindowPointer(HWND hwnd, void* pthis);
    void ClearWindowPointer(HWND hwnd);
}
#endif
