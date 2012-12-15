/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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
#ifndef IU_GUITOOLS_H
#define IU_GUITOOLS_H
#pragma once

#include "atlheaders.h"
#include <windows.h>

#define IS_CHECKED(ctrl) (SendDlgItemMessage(ctrl,BM_GETCHECK,0)==BST_CHECKED)

namespace GuiTools
{
	int AddComboBoxItem(HWND hDlg, int itemId, LPCTSTR item);
	bool AddComboBoxItems(HWND hDlg, int itemId, int itemCount, LPCTSTR item, ...);
   void GetCheck(HWND dlg, int id, bool& check);
	bool GetCheck(HWND dlg, int id);
   void SetCheck(HWND dlg, int id, bool check);
	void MakeLabelBold(HWND Label);
	void EnableNextN(HWND Control, int n, bool Enable);
	bool InsertMenu(HMENU hMenu, int pos, UINT id, const LPCTSTR szTitle,  HBITMAP bm=0);
	void FillRectGradient(HDC hdc, RECT FillRect, COLORREF start, COLORREF finish, bool Horizontal);
	bool SelectDialogFilter(LPTSTR szBuffer, int nMaxSize, int nCount, LPCTSTR szName, LPCTSTR szFilter,...);
	RECT GetDialogItemRect(HWND dialog, int itemId);
	void ShowDialogItem(HWND dlg, int itemId, bool show);
	void EnableDialogItem(HWND dlg, int itemId, bool enable);
	
	// Converts pixels to Win32 dialog units
	int dlgX(int WidthInPixels);
	int dlgY(int HeightInPixels);

	CString GetWindowText(HWND wnd);
	CString GetDlgItemText(HWND dialog, int controlId);

	HFONT MakeFontUnderLine(HFONT font);
	HFONT MakeFontBold(HFONT font);
	HFONT MakeFontSmaller(HFONT font);
	void MakeLabelItalic(HWND Label);

	int GetFontSize(int nFontHeight);
	int GetFontHeight(int nFontSize);

	int ScreenBPP();
	BOOL Is32BPP();

	bool IsChecked(HWND dlg, int id);

	CString SelectFolderDialog(HWND hWndParent, CString initialDir);
	RECT AutoSizeStaticControl(HWND control);

};
#endif