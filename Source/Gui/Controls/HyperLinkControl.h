/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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
#include "resource.h"
#include <atlcrack.h>

struct HyperLinkControlItem
{~HyperLinkControlItem(){DeleteObject(hIcon);}
    TCHAR szTitle[256];
    TCHAR szTip[256];
    HICON hIcon;
    int iconWidth;
    int iconHeight;
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
    int AddString(LPCTSTR szTitle,LPCTSTR szTip,int idCommand,HICON hIcon=NULL,bool Visible = true, int Align=0, bool LineBreak = false);
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
    int ScaleX(int x);
    int ScaleY(int y);
public:
    bool m_bHyperLinks;
    int NotifyParent(int nItem);
    CAtlArray<HyperLinkControlItem> Items;
    int BottomY, SubItemRightY;
    CFont BoldFont, UnderlineFont,BoldUnderLineFont;
    int Selected; 
    void SelectItem(int Index);
    bool CursorHand;
    HCURSOR HandCursor;
    CFont NormalFont;
    COLORREF m_BkColor;
    int dpiX;
    int dpiY;
};


