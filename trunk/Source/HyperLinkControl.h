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
#include "stdafx.h"
#include "resource.h"
#include <atlcrack.h>

struct HyperLinkControlItem
{
	TCHAR szTitle[256];
	TCHAR szTip[256];
	HICON hIcon;
	int idCommand;
	RECT ItemRect;
	bool Hover;
	bool Visible;
};

// CHyperLinkControl

class CHyperLinkControl :
	public CWindowImpl<CHyperLinkControl>, public CThemeImpl<CHyperLinkControl>
{
public:
	CHyperLinkControl();
	~CHyperLinkControl();
	DECLARE_WND_SUPERCLASS(_T("CHyperLinkControl"), CListBox::GetWndClassName())
	
    BEGIN_MSG_MAP(CHyperLinkControl)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkg)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_MOUSELEAVE(OnMouseLeave)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_KEYDOWN(OnKeyUp)
		MSG_WM_SETCURSOR(OnSetCursor)
    END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnDrawitem(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled);
	LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled);
	int AddString(LPTSTR szTitle,LPTSTR szTip,int idCommand,HICON hIcon=NULL,bool Visible = true, int Align=0, bool LineBreak = false);
	LRESULT OnMouseMove(UINT Flags, CPoint Pt);
	bool MouseSel,Track;
	LRESULT OnMouseLeave(void);
	LRESULT OnKillFocus(HWND hwndNewFocus);
	LRESULT OnSetFocus(HWND hwndOldFocus);
	LRESULT OnLButtonUp(UINT Flags, CPoint Pt);
	LRESULT OnKeyUp(TCHAR vk, UINT cRepeat, UINT flags);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
	void Init(COLORREF BkColor=RGB(255,255,255));
public:
	bool m_bHyperLinks;
	int NotifyParent(int nItem);
	CAtlArray<HyperLinkControlItem> Items;
	int BottomY, SubItemRightY;
	HFONT BoldFont, UnderlineFont,BoldUnderLineFont;
	int Selected;
	void SelectItem(int Index);
	bool CursorHand;
	HCURSOR HandCursor;
	HFONT NormalFont;
	COLORREF m_BkColor;
};


