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
#include "thread.h"
#include "ImageView.h"

// CThumbsView

struct ThumbsViewItem
{
	CString FileName;
	BOOL ThumbOutDate;
};


class CThumbsView :
	public CWindowImpl<CThumbsView, CListViewCtrl>,public CThreadImpl<CThumbsView>
{
public:
	
	CImageList ImageList;
	CThumbsView();
	~CThumbsView();
	DECLARE_WND_SUPERCLASS(_T("CThumbsView"), CListViewCtrl::GetWndClassName())
	
    BEGIN_MSG_MAP(CThumbsView)
		MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_BEGINDRAG, OnLvnBeginDrag)
		
    END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnLvnBeginDrag(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	bool m_NeedUpdate;
	CAutoCriticalSection ImageListCS;
	LRESULT OnMButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	int maxwidth,maxheight;
	void Init(bool Extended=false);
	int AddImage(LPCTSTR FileName, LPCTSTR Title, Image* Img=NULL);
	bool MyDeleteItem(int ItemIndex);
	int DeleteSelected();
	void UpdateImageIndexes(int StartIndex = 0);
	void MyDeleteAllItems();
	bool SimpleDelete(int ItemIndex, bool DeleteThumb = true);
	LPCTSTR GetFileName(int ItemIndex);
	LRESULT OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags);
	bool LoadThumbnail(int ItemID, Image *Img=NULL);
	int GetImageIndex(int ItemIndex);
	CImageView ImageView;
	LRESULT OnLButtonDblClk(UINT Flags, CPoint Pt);
	DWORD Run();
	void LoadThumbnails();
	void StopLoadingThumbnails();
	void ViewSelectedImage();
	bool ExtendedView;
	void OutDateThumb(int nIndex);
	void UpdateOutdated();
	void LockImagelist(bool bLock = true);
	bool StopAndWait();
	void SelectLastItem();
};


