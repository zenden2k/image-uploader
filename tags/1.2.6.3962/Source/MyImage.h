/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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

#pragma once
#include <atlcrack.h>
#include <gdiplus.h>
// CMyImage

class CMyImage :
	public CWindowImpl<CMyImage>
{
public:
	CMyImage();
	~CMyImage();
	DECLARE_WND_CLASS(_T("CMyImage"))
	
    BEGIN_MSG_MAP(CMyImage)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBkg)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_RBUTTONDOWN(OnLButtonDown)
		MSG_WM_MBUTTONUP(OnLButtonDown)
		MSG_WM_KEYDOWN(OnKeyDown)
    END_MSG_MAP()
	
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	Gdiplus::Bitmap *bm;
	bool IsImage;
	bool HideParent; // —пециально дл€ всплывающего окна просмотра
	LRESULT OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags);
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	bool LoadImage(LPCTSTR FileName,Gdiplus::Image *img=NULL, int ResourceID=0,bool Bitmap=false, COLORREF transp=0);
	LRESULT OnLButtonDown(UINT Flags, CPoint Pt);
	int ImageWidth, ImageHeight;
	HBITMAP BackBufferBm;
	HDC BackBufferDc;
	int BackBufferWidth, BackBufferHeight;
};