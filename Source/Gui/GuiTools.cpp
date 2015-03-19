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
#include <Func/WinUtils.h>
#include <Shobjidl.h>

namespace GuiTools
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

	bool GetCheck(HWND dlg, int id) {
		return ::SendDlgItemMessage(dlg, id,BM_GETCHECK,0,0) == BST_CHECKED;
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

	void MakeLabelItalic(HWND Label)
	{
		HFONT Font = reinterpret_cast<HFONT>(SendMessage(Label, WM_GETFONT,0,0));  

		if(!Font) return;

		LOGFONT alf;

		if(!(::GetObject(Font, sizeof(LOGFONT), &alf) == sizeof(LOGFONT))) return;

		alf.lfItalic = 1;

		HFONT NewFont = CreateFontIndirect(&alf);
		SendMessage(Label,WM_SETFONT,(WPARAM)NewFont,MAKELPARAM(false, 0));
	}

	void EnableNextN(HWND Control ,int n, bool Enable)
	{
		for(int i=0;i< n; i++)
		{
			Control = GetNextWindow(Control, GW_HWNDNEXT);
			EnableWindow(Control, Enable);
		}
	}

bool InsertMenu(HMENU hMenu, int pos, UINT id, const LPCTSTR szTitle,  HBITMAP bm){
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

void FillRectGradient(HDC hdc, RECT FillRect, COLORREF start, COLORREF finish, bool Horizontal) {
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

		//Начало прорисовки
		for (i = 0; i < n; i++) 
		{
			//Взависимости от того, кто мы - горизонтальный или вертикальный градиент
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
int dlgY(int HeightInPixels) {
	LONG units = GetDialogBaseUnits();
	short baseunitY = HIWORD(units);
	return HeightInPixels * baseunitY / 8;
}

CString GetWindowText(HWND wnd) {
	int len = GetWindowTextLength(wnd);
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
int GetFontSize(int nFontHeight) {
	return - MulDiv( nFontHeight, 72, GetDeviceCaps(::GetDC(0), LOGPIXELSY));
}

int GetFontHeight(int nFontSize) {
	return - MulDiv(nFontSize, GetDeviceCaps(::GetDC(0), LOGPIXELSY), 72);
}

HFONT GetSystemDialogFont()
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(NONCLIENTMETRICS);
#if (WINVER >= 0x0600)
	if ( !WinUtils::IsVista() ) {
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
	return (WinUtils::IsWinXP() & (ScreenBPP() >= 32));
}

typedef HRESULT  (STDAPICALLTYPE  *SHCreateItemFromParsingNameFuncType)(__in PCWSTR pszPath, __in_opt IBindCtx *pbc, __in REFIID riid, __deref_out void **ppv);


CString SelectFolderDialog(HWND hWndParent, CString initialDir){
	CString result;
	
	if ( WinUtils::IsVista() ) {
		static SHCreateItemFromParsingNameFuncType SHCreateItemFromParsingNameFunc = 0;
		if ( !SHCreateItemFromParsingNameFunc ) {
			HMODULE module = LoadLibrary(_T("shell32.dll"));
			SHCreateItemFromParsingNameFunc = (SHCreateItemFromParsingNameFuncType)GetProcAddress(module,"SHCreateItemFromParsingName");

		}
		// CoCreate the File Open Dialog object.
		IFileDialog *pfd = NULL;
		IShellItem *pInitDir = NULL;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));

		if ( SUCCEEDED(hr) ) {
			// Set the options on the dialog.
			DWORD dwFlags;

			// Before setting, always get the options first in order 
			// not to override existing options.
			hr = pfd->GetOptions(&dwFlags);
			if (SUCCEEDED(hr)) {
				if (!initialDir.IsEmpty()){
					SHCreateItemFromParsingNameFunc(initialDir, NULL, __uuidof(IShellItem), (LPVOID *)&pInitDir);
					pfd->SetFolder(pInitDir);
				}
				// In this case, get shell items only for file system items.
				hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);
				if (SUCCEEDED(hr)) {
					// Show the dialog
					hr = pfd->Show(NULL);
					if (SUCCEEDED(hr)) {
						// Obtain the result once the user clicks 
						// the 'Open' button.
						// The result is an IShellItem object.
						IShellItem *psiResult;
						hr = pfd->GetResult(&psiResult);
						if (SUCCEEDED(hr))
						{
							// We are just going to print out the 
							// name of the file for sample sake.
							PWSTR pszFilePath = NULL;
							hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, 
								&pszFilePath);
							if (SUCCEEDED(hr))
							{

								result = pszFilePath;
								CoTaskMemFree(pszFilePath);
							}
							psiResult->Release();
						}
					}
				}
			}

			pfd->Release();
		}
		
	} else {
		CFolderDialog fd(hWndParent,TR("Выбор папки"), BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE );
		if ( !initialDir.IsEmpty() ) {
			fd.SetInitialFolder(initialDir, true);
		}
		if(fd.DoModal(hWndParent) == IDOK) {
			return  fd.GetFolderPath();
		}
	}
	return result;
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
	HDC dc = ::GetDC(control);
	HFONT font = (HFONT) SendMessage(control, WM_GETFONT, 0, 0);

	SIZE textSize;
	HGDIOBJ oldFont = SelectObject(dc, font);
	GetTextExtentPoint32(dc, text, text.GetLength(), &textSize);
	SetWindowPos(control,0, 0,0, textSize.cx, textSize.cy, SWP_NOMOVE );
	RECT controlRect;
	GetWindowRect(control, &controlRect);
	MapWindowPoints(0 /*means desktop*/, GetParent(control), reinterpret_cast<LPPOINT>(&controlRect), 2);
	SelectObject(dc, oldFont);
	ReleaseDC(control, dc);
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
	ScreenToClient(Parent, (LPPOINT)&WindowRect);
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

};