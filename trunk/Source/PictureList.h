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

#pragma once

#include <atlcoll.h>
// CPictureLists
struct CPictureItem
{
	Image *img;
	LPTSTR Title;
	bool selected;
	LPTSTR filename;
};

class CPictureList :
	public CWindowImpl<CPictureList>
{
public:
	CPictureList();
	~CPictureList();
	DECLARE_WND_CLASS(_T("CPictureList"))
	
    BEGIN_MSG_MAP(CPictureList)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBkg)
		
MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
MESSAGE_HANDLER(WM_MOUSEWHEEL/*0x020A*/, OnMouseWheel)
MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
    END_MSG_MAP()
	CAtlArray<CPictureItem> ImageList;

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	Bitmap *bm,*BackBuffer;
	//bool IsImage;
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
public:
	bool AddPicture(LPTSTR FileName, LPTSTR Title = NULL, Image *img =NULL,DWORD Data = 0);
public:
	void BufferOutdated(void);

public:
	void UpdateScrollbars(void);
	LRESULT OnVScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnHScroll(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


int maxwidth,maxheight;
	int thumbwidth,thumbheight,hmargin,vmargin,vdist,hdist,VScrollPosition,BufferWidth,BufferHeight,thumblabel;
};


