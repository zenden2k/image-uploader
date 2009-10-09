/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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

// CColorPicker

class CColorPicker :
	public CWindowImpl<CColorPicker>
{
public:
	CColorPicker();
	~CColorPicker();
	DECLARE_WND_CLASS(_T("CColorPicker"))
	
    BEGIN_MSG_MAP(CColorPicker)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBkg)
		MESSAGE_HANDLER( WM_KEYDOWN, OnKeyDown)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
    END_MSG_MAP()
	
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	
	COLORREF Color;

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnLButtonDown(UINT Flags, CPoint Pt);
	CColorDialog ColorDialog;
	BOOL ChooseColor();
};


