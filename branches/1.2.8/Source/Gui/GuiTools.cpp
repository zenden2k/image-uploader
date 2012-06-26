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

#include "Gui/GuiTools.h"

namespace ZGuiTools
{
	int AddComboBoxItem(HWND hDlg, int itemId, LPCTSTR item)
	{
		return ::SendDlgItemMessage(hDlg, itemId, CB_ADDSTRING, 0, (LPARAM)item);
	}

	bool AddComboBoxItems(HWND hDlg, int itemId, int itemCount, LPCTSTR item, ...)
	{
		bool result = true;
		for(int i=0; i<itemCount; i++)
		{
			if(AddComboBoxItem(hDlg, itemId, *(&item + i)) < 0)
				result = false;
		}
		return result;
	}

   bool IsChecked(HWND dlg, int id)
   {
      return  ::SendDlgItemMessage(dlg, id,BM_GETCHECK,0,0) == BST_CHECKED;
   }

   void  GetCheck(HWND dlg, int id, bool& check)
   {
      check = ::SendDlgItemMessage(dlg, id,BM_GETCHECK,0,0)==BST_CHECKED;
   }

   void  SetCheck(HWND dlg, int id, bool check)
   {
      ::SendDlgItemMessage(dlg, id,BM_SETCHECK, check,0);
   }

	void MakeLabelBold(HWND Label)
	{
		HFONT Font = reinterpret_cast<HFONT>(SendMessage(Label, WM_GETFONT,0,0));  

		if(!Font) return;

		LOGFONT alf;

		if(!(::GetObject(Font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT))) return;

		alf.lfWeight = FW_BOLD;

		HFONT NewFont = CreateFontIndirect(&alf);
		SendMessage(Label,WM_SETFONT,(WPARAM)NewFont,MAKELPARAM(false, 0));
		alf.lfHeight = -MulDiv(13, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);
	}

	void EnableNextN(HWND Control ,int n, bool Enable)
	{
		for(int i=0;i< n; i++)
		{
			Control = GetNextWindow(Control, GW_HWNDNEXT);
			EnableWindow(Control, Enable);
		}
	}

	bool IUInsertMenu(HMENU hMenu, int pos, UINT id, const LPCTSTR szTitle,  HBITMAP bm)
	{
		MENUITEMINFO MenuItem;

		MenuItem.cbSize = sizeof(MenuItem);
		if(szTitle)
			MenuItem.fType = MFT_STRING;
		else MenuItem.fType = MFT_SEPARATOR;
		MenuItem.fMask = MIIM_TYPE	| MIIM_ID | MIIM_DATA;
		if(bm)
			MenuItem.fMask |= MIIM_CHECKMARKS;
		MenuItem.wID = id;
		MenuItem.hbmpChecked = bm;
		MenuItem.hbmpUnchecked = bm;
		MenuItem.dwTypeData = (LPWSTR)szTitle;
		return InsertMenuItem(hMenu, pos, TRUE, &MenuItem)!=0;
	}

	void FillRectGradient(HDC hdc, RECT FillRect, COLORREF start, COLORREF finish, bool Horizontal)
	{
		RECT rectFill;          
		float fStep;            //The size of each band in pixels
		HBRUSH hBrush;
		int i;  // Loop index

		float r, g, b;
		int n = 256;
		//FillRect.bottom--;
		COLORREF c;

		if(!Horizontal)
			fStep = (float)(FillRect.bottom - FillRect.top) / 256;
		else 
			fStep = (float)(FillRect.right - FillRect.left) / 256;

		if( fStep < 1)
		{
			fStep = 1;
			if(!Horizontal)
				n = FillRect.bottom - FillRect.top;
			else 
				n = (FillRect.right - FillRect.left);
		}

		r = (float)(GetRValue(finish)-GetRValue(start))/(n-1);

		g = (float)(GetGValue(finish)-GetGValue(start))/(n-1);

		b = (float)(GetBValue(finish)-GetBValue(start))/(n-1);

		//Ќачало прорисовки
		for (i = 0; i < n; i++) 
		{
			//¬зависимости от того, кто мы - горизонтальный или вертикальный градиент
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

	bool SelectDialogFilter(LPTSTR szBuffer, int nMaxSize, int nCount, LPCTSTR szName, LPCTSTR szFilter,...)
	{
		*szBuffer = 0;
		LPCTSTR *pszName, *pszFilter;
		pszName = &szName;
		pszFilter = &szFilter; 

		for(int i=0; i<nCount; i++)
		{
			int nLen = lstrlen(*pszName);
			lstrcpy(szBuffer, *pszName);
			szBuffer[nLen]=0;
			szBuffer+=nLen+1;

			nLen = lstrlen(*pszFilter);
			lstrcpy(szBuffer, *pszFilter);
			szBuffer[nLen]=0;
			szBuffer+=nLen+1;
			pszName+=2;
			pszFilter+=2;
		}
		*szBuffer=0;
		return true;
	}

	// Converts pixels to Win32 dialog units
	int dlgX(int WidthInPixels)
	{
		LONG units = GetDialogBaseUnits();
		short baseunitX = LOWORD(units);
		return WidthInPixels * baseunitX / 4;
	}

	// Converts pixels to Win32 dialog units
	int dlgY(int HeightInPixels)
	{
		LONG units = GetDialogBaseUnits();
		short baseunitY = HIWORD(units);
		return HeightInPixels * baseunitY / 8;
	}


	CString IU_GetWindowText(HWND wnd)
	{
		int len = GetWindowTextLength(wnd);
		CString buf;
		GetWindowText(wnd, buf.GetBuffer(len + 1), len + 1);
		buf.ReleaseBuffer(-1);
		return buf;
	}

};