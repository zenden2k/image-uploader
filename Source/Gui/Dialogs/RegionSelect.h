/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#pragma once

#include <memory>
#include "atlheaders.h"
#include <atlcrack.h>
#include "resource.h"       // main symbols
#include "Core/ScreenCapture.h"

class CRegionSelectCallback
{
public: 
    virtual void OnScreenshotFinished(int Result)=0;
    virtual void OnScreenshotSaving(LPTSTR FileName, Gdiplus::Bitmap* Bm)=0;
};

enum class SelectionMode {smRectangles, smFreeform, smWindowHandles };

class CRegionSelect: public CWindowImpl<CRegionSelect>
{
    public:
        CRegionSelect();
        ~CRegionSelect() override;

        bool wasImageEdited() const;
        std::shared_ptr<ScreenCapture::CScreenshotRegion> region() const;
        bool Execute(HBITMAP screenshot, int width, int height, HMONITOR monitor = nullptr);
        void setSelectionMode(SelectionMode selMode, bool onlyTopWindows = false);
        SelectionMode selectionMode() const;

        DECLARE_WND_CLASS(_T("CRegionSelect"))

    protected:
        BEGIN_MSG_MAP(CRegionSelect)
            MESSAGE_HANDLER(WM_NCCREATE, OnNcCreate)
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
        LRESULT OnNcCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

        LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnEraseBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);

        bool setDrawingParams(COLORREF color, int brushSize);

        CBitmap m_bmScreenShot;
        SelectionMode m_SelectionMode;
        HWND hSelWnd;
        RECT m_PrevWindowRect;
        POINT Start,End;
        bool Down;
        std::vector<POINT> m_curvePoints;
        CPoint topLeft;
        bool m_bSaveAsRegion;
        bool m_btoolWindowTimerRunning;
        HPEN pen;
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
        //HDC dstDC_;
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
        std::shared_ptr<ScreenCapture::CScreenshotRegion> m_ResultRegion;
        bool m_bPictureChanged;
        CToolBarCtrl Toolbar;
        ScreenCapture::CWindowHandlesRegion m_SelectedWindowsRegion;
        int lineType;
        CRect m_screenBounds;
        bool onlyTopWindows_ = false;
};

extern CRegionSelect RegionSelect;
