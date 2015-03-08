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

#include "atlheaders.h"
#include <atlcrack.h>
#include "resource.h"       // main symbols
#include "Gui/Controls/myimage.h"
#include "Core/ScreenCapture.h"

class CRegionSelectCallback
{
public: 
	virtual void OnScreenshotFinished(int Result)=NULL;
	virtual void OnScreenshotSaving(LPTSTR FileName, Gdiplus::Bitmap* Bm)=NULL;
};

enum SelectionMode {smRectangles, smFreeform, smWindowHandles };

class CRegionSelect: public CWindowImpl<CRegionSelect>
{
	public:
		CRegionSelect();
		~CRegionSelect();
		CRect m_screenBounds;
		bool wasImageEdited();
		CScreenshotRegion* region() const;
		bool Execute(HBITMAP screenshot, int width, int height);

		DECLARE_WND_CLASS(_T("CRegionSelect"))

	protected:
		BEGIN_MSG_MAP(CRegionSelect)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBg)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
			MESSAGE_HANDLER(WM_RBUTTONDOWN, OnMouseMove)
			MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
			MESSAGE_HANDLER(WM_CHAR, OnChar)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MSG_WM_SETCURSOR(OnSetCursor)
			MSG_WM_KEYDOWN(OnKeyDown)
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnEraseBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKillFocus(HWND hwndNewFocus);
		void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
		BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);

		bool setDrawingParams(COLORREF color, int brushSize);

		CBitmap m_bmScreenShot;
		SelectionMode m_SelectionMode;
		HWND hSelWnd;
		RECT m_PrevWindowRect;
		void ShowW(HWND Parent, HBITMAP bmp,int w,int h);
		POINT Start,End;
		bool Down;
		std::vector<POINT> m_curvePoints;
		CPoint topLeft;
		bool m_bSaveAsRegion;
		bool m_btoolWindowTimerRunning;
		HPEN pen;
		HWND Parent;
		HCURSOR CrossCursor ;
		bool m_bPainted;
		HCURSOR HandCursor ;
		
		bool m_bDocumentChanged;
		void Finish();
		void Cleanup();
		bool m_bFinish;
		CBitmap doubleBm;
		CDC doubleDC;
		Gdiplus::Bitmap *m_DoubleBuffer;
		int RectCount;
		Gdiplus::Bitmap *gdipBm ;
		HPEN DrawingPen;
		HBRUSH DrawingBrush;
		HDC dstDC;
		int cxOld, cyOld;
		int m_brushSize;
		int m_Width;
		int m_Height;
		bool m_bResult;
		COLORREF m_brushColor;
		CRgn m_SelectionRegion;
		CDC memDC2;
		CDC alphaDC; CBitmap alphaBm;
		CRgn m_prevWindowRgn;
		CScreenshotRegion * m_ResultRegion;
		bool m_bPictureChanged;
		CToolBarCtrl Toolbar;
		CWindowHandlesRegion m_SelectedWindowsRegion;
		int lineType;

};

extern CRegionSelect RegionSelect;