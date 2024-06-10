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

#ifndef MYIMAGE_H
#define MYIMAGE_H

#pragma once
#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
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
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBkg)
        MESSAGE_HANDLER( WM_KEYDOWN, OnKeyDown)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_RBUTTONDOWN(OnLButtonDown)
        MSG_WM_MBUTTONUP(OnLButtonDown)
    END_MSG_MAP()
    
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    Gdiplus::Bitmap *bm_;
    bool imageLoaded_;
    bool HideParent;
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    bool loadImage(LPCTSTR FileName,Gdiplus::Image *img=NULL, int ResourceID=0,bool Bitmap=false, COLORREF transp=0, bool allowEnlarge = false, bool whiteBg = false);
    LRESULT OnLButtonDown(UINT Flags, CPoint Pt);
    int imageWidth_, imageHeight_;
    HBITMAP BackBufferBm;
    HGDIOBJ oldBm_ = nullptr;
    HDC BackBufferDc;
    int BackBufferWidth, BackBufferHeight;
};

#endif // MYIMAGE_H
