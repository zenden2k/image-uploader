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

#include "resource.h"       // main symbols
#include "myimage.h"
#include <atlcrack.h>
// CRegionSelect

class CRegionSelectCallback
{
public: 
	virtual void OnScreenshotFinished(int Result)=NULL;
	virtual void OnScreenshotSaving(LPTSTR FileName, Bitmap* Bm)=NULL;
};

class CRegionSelect : 
	public CDialogImpl<CRegionSelect>	
{
	public:
		CRegionSelect();

		~CRegionSelect();

		CRegionSelectCallback *m_pCallBack;
	HANDLE m_hTimerQueue, m_hTimer;
	TCHAR m_szFileName[MAX_PATH];
	CWizardDlg *m_pWizardDlg;

	bool Execute(CRegionSelectCallback *RegionSelectCallback = NULL);
	static VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
	
	void Finish();


		enum { IDD = IDD_IMAGEVIEW };

    BEGIN_MSG_MAP(CRegionSelect)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBg)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_MBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_ACTIVATE(OnActivate)
		MSG_WM_SETCURSOR(OnSetCursor)
		MSG_WM_KEYDOWN(OnKeyDown)
      COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedOK)
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(HWND hwndNewFocus);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
	
public:
	void OnTimer(UINT_PTR nIDEvent);
	//bool ViewImage(LPTSTR FileName, HWND Parent=NULL);
	LRESULT OnActivate(UINT state, BOOL fMinimized, HWND hwndActDeact);
	HBITMAP bm;
	void ShowW(HWND Parent, HBITMAP bmp,int w,int h);
	POINT Start,End;
	bool Down;
	HPEN pen;
	HWND Parent;
	HBITMAP oldbm;
	HBITMAP oldbm2;
	HCURSOR CrossCursor ;
private:
	SIZE sz;
	HDC memDC;	HDC memDC2;
	HDC dstDC;
	void Hide(bool Res = true);
};

extern CRegionSelect RegionSelect;


