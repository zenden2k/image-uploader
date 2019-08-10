/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
#include "atlheaders.h"
#include <atlcrack.h>

struct HyperLinkControlItem
{
    ~HyperLinkControlItem() {
        DeleteObject(hIcon);
    }
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
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MSG_WM_MOUSEMOVE(OnMouseMove)
        MSG_WM_MOUSELEAVE(OnMouseLeave)
        MSG_WM_KILLFOCUS(OnKillFocus)
        MSG_WM_SETFOCUS(OnSetFocus)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_SETCURSOR(OnSetCursor)
        MESSAGE_HANDLER_EX(WM_GETOBJECT, OnGetObject)
        MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
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
    LRESULT OnLButtonDown(UINT Flags, CPoint Pt);
    LRESULT OnLButtonUp(UINT Flags, CPoint Pt);
    LRESULT OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    BOOL OnSetCursor(CWindow wnd, UINT nHitTest, UINT message);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGetObject(UINT, WPARAM wParam, LPARAM lParam);
    LRESULT OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    void Init(COLORREF BkColor=RGB(255,255,255));
    size_t ItemCount() const;
    CString GetItemTitle(size_t item) const;
    CString GetItemDescription(size_t item) const;
    CRect GetItemRect(size_t itemIndex) const;
    int SelectedIndex() const;
    int ItemFromPoint(POINT pt) const;
    int ScaleX(int x) const;
    int ScaleY(int y) const;
    bool m_bHyperLinks;
    int NotifyParent(int nItem);
    void SelectItem(int Index);
    void HoverItem(int Index);
    HyperLinkControlItem* getItemByCommand(int command);
    int selectedItemIndex() const;
protected:
    CAtlArray<HyperLinkControlItem> Items;
    int BottomY, SubItemRightY;
    CFont BoldFont, UnderlineFont,BoldUnderLineFont;
    int selectedItemIndex_;
    int hoverItemIndex_;
    bool CursorHand;
    HCURSOR HandCursor;
    CFont NormalFont;
    COLORREF m_BkColor;
    int dpiX;
    int dpiY;
    int mouseDownItemIndex_;
    CComPtr<IAccessible> acc_;
    static int GetTextWidth(HDC dc, LPCTSTR Text, HFONT Font);
};


