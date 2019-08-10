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

#include "HyperLinkControl.h"

#include <algorithm>

#include "Gui/GuiTools.h"
#include "HyperLinkControlAccessible.h"

// CHyperLinkControl
CHyperLinkControl::CHyperLinkControl()
{
    MouseSel = false;
    Track = false;
    BottomY = 1;
    SubItemRightY = -1;
    selectedItemIndex_ = -1;
    mouseDownItemIndex_ = -1;
    hoverItemIndex_ = -1;
    CursorHand = false;
    m_bHyperLinks = true;
    m_BkColor = RGB(0, 0, 0);
    HandCursor = LoadCursor(NULL, IDC_HAND); // Loading "Hand" cursor into memory
    SetThemeClassList ( L"globals" );
    HDC hdc = ::GetDC(NULL);
    if (hdc) {
        dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ::ReleaseDC(NULL, hdc);
    }
}

CHyperLinkControl::~CHyperLinkControl()
{

}

/* CHyperLinkControl::Init
-----------------------
Must be called before adding any items 
*/
void CHyperLinkControl::Init(COLORREF BkColor)
{
    NormalFont = GetFont();
    BoldFont = GuiTools::MakeFontBold(NormalFont);
    UnderlineFont = GuiTools::MakeFontUnderLine(NormalFont);
    BoldUnderLineFont = GuiTools::MakeFontUnderLine(BoldFont);
    m_BkColor = BkColor;
    OpenThemeData();
}

size_t CHyperLinkControl::ItemCount() const {
    return Items.GetCount();
}

CString CHyperLinkControl::GetItemTitle(size_t item) const {
    if (item >= 0 && item < Items.GetCount()) {
        return Items[item].szTitle; 
    }
    return CString();
}

CString CHyperLinkControl::GetItemDescription(size_t item) const {
    if (item < Items.GetCount()) {
        return Items[item].szTip;
    }
    return CString();
}

CRect CHyperLinkControl::GetItemRect(size_t itemIndex) const {
    if (itemIndex < Items.GetCount()) {
        return Items[itemIndex].ItemRect;
    }
    return {};
}

int CHyperLinkControl::SelectedIndex() const {
    return selectedItemIndex_;
}

int CHyperLinkControl::ItemFromPoint(POINT pt) const {
    RECT ClientRect;
    GetClientRect(&ClientRect);

    for (size_t i = 0; i< Items.GetCount(); i++) {
        if (!Items[i].Visible) {
            continue;
        }
        CRect rc(Items[i].ItemRect);
        if (!m_bHyperLinks && *Items[i].szTip) {
            rc.right = ClientRect.right - 6;
            rc.InflateRect(3, 6);
        }

        if (rc.PtInRect(pt)) {
            return i;
        }
    }
    return -1;
}

int CHyperLinkControl::GetTextWidth(HDC dc, LPCTSTR Text, HFONT Font)
{
    SIZE sz;
    HGDIOBJ  OldFont = SelectObject(dc, Font);
    GetTextExtentPoint32(dc, Text, lstrlen(Text), &sz);
    SelectObject(dc, OldFont);
    return sz.cx;
}

int CHyperLinkControl::AddString(LPCTSTR szTitle,LPCTSTR szTip,int idCommand,HICON hIcon,bool Visible,int Align,  bool LineBreak)
{
    // TODO: This shit should be rewritten from scratch
    RECT ClientRect;
    GetClientRect(&ClientRect);

    HyperLinkControlItem * item = new  HyperLinkControlItem;
    if(szTip)
        lstrcpy(item->szTip,szTip);
    else 
        *(item->szTip)=0;
    lstrcpy(item->szTitle, szTitle);
    item->hIcon = hIcon;
    if ( hIcon ) {
        GuiTools::IconInfo iconInfo = GuiTools::GetIconInfo(hIcon);
        item->iconWidth = iconInfo.nWidth;
        item->iconHeight = iconInfo.nHeight;
    }
    item->idCommand=idCommand;
    item->Hover=false;
    item->Visible=Visible;
    CWindowDC dc(m_hWnd);

    int szTipWidth = szTip ? GetTextWidth(dc, szTip, NormalFont) : 0;
    int TitleWidth = std::max(GetTextWidth(dc, szTitle, BoldFont), szTipWidth);

    if(szTip)
    {
        if(SubItemRightY!= -1)
            BottomY+= 16;
        item->ItemRect.left = 5;
        item->ItemRect.top = BottomY +(m_bHyperLinks?ScaleY(15):ScaleY(10));
        item->ItemRect.right=ScaleX(10)+item->ItemRect.left+ item->iconWidth + TitleWidth+1/*ClientRect.right*/;
        item->ItemRect.bottom = item->ItemRect.top+ScaleY(33);
        BottomY+= ScaleY(10+38);
        SubItemRightY= -1;
    }
    else
    {    

        int TextWidth = GetTextWidth(dc, szTitle, NormalFont);
        if(Align==2) // right aligned text
        {
            SubItemRightY = ClientRect.right - TextWidth - 3 -35;
        }
        else 
        {
            if(SubItemRightY== -1)
                SubItemRightY=35;
            else 
                SubItemRightY+=ScaleY(20);
        }
        if(  LineBreak)
        {
            //item->ItemRect.left = 35;
            SubItemRightY =35;
            BottomY+=ScaleY(20);
            //item->ItemRect.top = BottomY;
            
        }
        //else
        {

        
        item->ItemRect.left = SubItemRightY;
        item->ItemRect.top = BottomY + ((BottomY>1)?6:0);
        }
        if(!m_bHyperLinks) 
            item->ItemRect.top+=ScaleY(5);
        item->ItemRect.right = 1+GetTextWidth(dc, szTitle, NormalFont)+ScaleX(23)+item->ItemRect.left;
        item->ItemRect.bottom = item->ItemRect.top+ScaleY(20);
        SubItemRightY+=item->ItemRect.right-item->ItemRect.left;
    }
    Items.Add(*item);
    delete item;
    return TRUE;
}

LRESULT CHyperLinkControl::OnMouseMove(UINT Flags, CPoint Pt)
{
    RECT ClientRect;
    GetClientRect(&ClientRect);

    if(!Track) // Capturing mouse
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        ::_TrackMouseEvent(&tme); // We want to receive WM_MOUSELEAVE message
        Track = true;
    }

    for(size_t i=0;i< Items.GetCount(); i++)
    {
        if(!Items[i].Visible) continue;
        CRect rc(Items[i].ItemRect);
        if(!m_bHyperLinks && *Items[i].szTip)
        {
            rc.right= ClientRect.right - 6;
            rc.InflateRect(3,6);
        }

        if(rc.PtInRect(Pt))
        {
            if(m_bHyperLinks  || !*Items[i].szTip)
            {
                if(!CursorHand)
                    SetCursor(HandCursor);  // we need this because system doesn't send WM_CURSOR message immediately
            }
            if(static_cast<size_t>(hoverItemIndex_) == i ) return 0;
            Items[i].Hover = true;
            HoverItem(i);

            return 0;
        }
    }

    HoverItem(-1);

    return 0;
}

LRESULT CHyperLinkControl::OnMouseLeave(void)
{
    HoverItem(-1);
    Track = false;

    return 0;
}

LRESULT CHyperLinkControl::OnKillFocus(HWND hwndNewFocus)
{
    SelectItem(-1);
    return 0;
}

LRESULT CHyperLinkControl::OnSetFocus(HWND hwndOldFocus)
{
    if (selectedItemIndex_ != -1) {
        return 0;
    }

    /*if (hwndOldFocus == ::GetNextDlgTabItem(::GetParent(GetParent()), m_hWnd, true)
        || hwndOldFocus == ::GetNextDlgTabItem(::GetParent(GetParent()), m_hWnd, false)) {
        SelectItem(0);
    }*/

    return 0;
}

LRESULT CHyperLinkControl::OnLButtonDown(UINT Flags, CPoint Pt) {
    RECT ClientRect;
    GetClientRect(&ClientRect);
    mouseDownItemIndex_ = -1;
    for (size_t i = 0; i < Items.GetCount(); i++)
    {
        const auto& item = Items[i];
        if (!item.Visible) {
            continue;
        }
        CRect rc(item.ItemRect);

        if (!m_bHyperLinks && *item.szTip) {
            rc.right = ClientRect.right - 6;
            rc.InflateRect(3, 6);
        }

        if (rc.PtInRect(Pt)) {
            mouseDownItemIndex_ = i;
            break;
        }
    }
    return 0;
}

LRESULT CHyperLinkControl::OnLButtonUp(UINT Flags, CPoint Pt)
{
    RECT ClientRect;
    GetClientRect(&ClientRect);

    for(size_t i=0;i< Items.GetCount(); i++)
    {
        const auto& item = Items[i];
        if(!item.Visible) continue;
        CRect rc(item.ItemRect);

        if(!m_bHyperLinks && *item.szTip){
            rc.right= ClientRect.right - 6;
            rc.InflateRect(3,6);
        }

        if(rc.PtInRect(Pt)) {
            if (mouseDownItemIndex_ != -1 && mouseDownItemIndex_ == i) {
                NotifyParent(i);
                mouseDownItemIndex_ = -1;
            }
            break;
        }
    }

    return 0;
}

LRESULT CHyperLinkControl::OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags)
{
    if (vk == VK_DOWN || (vk == VK_TAB && !(GetKeyState(VK_SHIFT) & 0x80)))
    {
        bool itemSelected = false;
        for (size_t j = selectedItemIndex_ + 1; j < Items.GetCount(); j++) {
            if (Items[j].Visible) {
                SelectItem(j);
                itemSelected = true;
                break;
            }
        }
        if (!itemSelected) {
            // Set focus to next dialog control
            ::PostMessage(GetParent(), WM_NEXTDLGCTL, 0, FALSE);
        }
    } else if (vk == VK_UP || (vk == VK_TAB && (GetKeyState(VK_SHIFT) & 0x80))) {
        bool itemSelected = false;
        for (int j = selectedItemIndex_ - 1; j >=0 ; j--) {
            if (Items[j].Visible) {
                SelectItem(j);
                itemSelected = true;
                break;
            }
        }
        if (!itemSelected) {
            // Set focus to previous dialog control
            ::PostMessage(GetParent(), WM_NEXTDLGCTL, 1, FALSE);
        }
    }

    if (vk == VK_RETURN || vk == VK_SPACE)
    {
        if (selectedItemIndex_ != -1) {
            NotifyParent(selectedItemIndex_);
        }
    }
    return 0;
}

int CHyperLinkControl::NotifyParent(int nItem)
{
    if(nItem<0) return 0;
    ::SendMessage(GetParent(), WM_COMMAND, (WPARAM)MAKELONG(Items[nItem].idCommand,BN_CLICKED), (LPARAM)m_hWnd);
    SelectItem(-1);
    return 0;
}

LRESULT CHyperLinkControl::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CPaintDC dc(m_hWnd);
    RECT rc;
    GetClientRect(&rc);

    
    CRect r(rc);
    CBrush br;
    //br.CreateSolidBrush(RGB(0,0,0));
    br.CreateSolidBrush(m_BkColor);
    dc.FillRect(r,br);

    dc.SetBkMode(TRANSPARENT);

    for(size_t i=0; i<Items.GetCount(); i++)
    {
        const HyperLinkControlItem& item = Items[i];
        if (!item.Visible) {
            continue;
        }
        bool isHighlighted = static_cast<size_t>(selectedItemIndex_) == i || hoverItemIndex_ == i;
        dc.SetTextColor(RGB(0,0,0));

        if(*(item.szTip)) // If we draw "big" item (with tip)
        {
            RECT TextRect = item.ItemRect;
            
            if(!m_bHyperLinks && (isHighlighted))
            {    
                CRect rec = item.ItemRect;
                rec.right = rc.right-6;
                rec.InflateRect(3,6);
                GuiTools::FillRectGradient(dc.m_hDC, rec, 0xEAE2D9, 0xD3C1AF, false);
            }

            TextRect.left += item.iconWidth + ScaleX(5);
            if(!m_bHyperLinks){
                TextRect.left +=ScaleX(20);
                TextRect.right +=ScaleX(20);
            }
            HFONT oldFont;
            if (isHighlighted && m_bHyperLinks) {
                oldFont = dc.SelectFont(BoldUnderLineFont);
            }
            else {
                oldFont = dc.SelectFont(BoldFont);
            }

            int textY = item.ItemRect.top;
            if(*item.szTip == _T(' '))
            {
                SIZE TextDims;
                GetTextExtentPoint32(dc, item.szTitle, lstrlen(item.szTitle),    &TextDims);
                textY += (33-TextDims.cy)/2;

            }

            ExtTextOutW(dc.m_hDC, TextRect.left, textY, ETO_CLIPPED, &TextRect, item.szTitle, wcslen(item.szTitle), 0);
            dc.SelectFont(oldFont);

            if (isHighlighted && m_bHyperLinks) {
                oldFont = dc.SelectFont(UnderlineFont);
            } else {
                oldFont = dc.SelectFont(GetFont());
            }
            dc.SetTextColor(RGB(100,100,100));

            ExtTextOutW(dc.m_hDC, TextRect.left, item.ItemRect.top+18, ETO_CLIPPED, &TextRect, item.szTip, wcslen(item.szTip), 0);

            dc.SetBkMode(TRANSPARENT);
            if(m_bHyperLinks){
                //dc.DrawIconEx(item.ItemRect.left, item.ItemRect.top + 2, item.hIcon, 48, 48);
                dc.DrawIcon(item.ItemRect.left+1,item.ItemRect.top+2,item.hIcon);
            }
            else dc.DrawIcon(item.ItemRect.left+15,item.ItemRect.top+2,item.hIcon);
            dc.SelectFont(oldFont);
        } // End of drawing "big" item

        else 
        {
            HFONT oldFont;
            if (isHighlighted) { // If item we draw is selected
                oldFont = dc.SelectFont(UnderlineFont);
            } else {
                oldFont = dc.SelectFont(GetFont());
            }

            dc.DrawIconEx(item.ItemRect.left,item.ItemRect.top+1,item.hIcon,16,16);
                
            dc.ExtTextOut(item.ItemRect.left+23, item.ItemRect.top+1, ETO_CLIPPED/*|ETO_OPAQUE/**/, &item.ItemRect, item.szTitle, wcslen(item.szTitle), 0);
            dc.SelectFont(oldFont);
        }
    }

    //EndPaint(&ps);
    return 0;
}

LRESULT CHyperLinkControl::OnEraseBkg(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true; 
    return 1;
}

void CHyperLinkControl::SelectItem(int Index)
{
    if(Index == selectedItemIndex_) return; 

    RECT ClientRect;
    GetClientRect(&ClientRect);

    if(selectedItemIndex_ != -1) // Repainting previous selection
    {
        CRect temp = Items[selectedItemIndex_].ItemRect;
        if(!m_bHyperLinks)
        {
            temp.right= ClientRect.right;
            temp.InflateRect(3,6);
        }
        if(*Items[selectedItemIndex_].szTip)
            //temp.left+=40;
            InvalidateRect(&temp,false);
        else InvalidateRect(&Items[selectedItemIndex_].ItemRect,false);
    }

    selectedItemIndex_ = Index;

    if(selectedItemIndex_!=-1)  //Repainting selected hyperlink
    {
        CRect temp = Items[Index].ItemRect;
        if(!m_bHyperLinks)
        {
            temp.right= ClientRect.right;
            temp.InflateRect(3,6);
        }
        if(*Items[Index].szTip)
            //temp.left+=40;
            InvalidateRect(&temp,false);
        else InvalidateRect(&Items[Index].ItemRect,false);
        NotifyWinEvent(EVENT_OBJECT_FOCUS, m_hWnd, OBJID_CLIENT, Index + 1);
    }
}

void CHyperLinkControl::HoverItem(int Index)
{
    if (Index == hoverItemIndex_) {
        return;
    }

    RECT ClientRect;
    GetClientRect(&ClientRect);

    if (hoverItemIndex_ != -1) // Repainting previous selection
    {
        const auto& item = Items[hoverItemIndex_];
        CRect temp = item.ItemRect;
        if (!m_bHyperLinks)
        {
            temp.right = ClientRect.right;
            temp.InflateRect(3, 6);
        }
        if (*Items[hoverItemIndex_].szTip)
            InvalidateRect(&temp, false);
        else InvalidateRect(&Items[hoverItemIndex_].ItemRect, false);
    }

    hoverItemIndex_ = Index;

    if (hoverItemIndex_ != -1)  //Repainting selected hyperlink
    {
        const auto& item = Items[Index];
        CRect temp = item.ItemRect;
        if (!m_bHyperLinks)
        {
            temp.right = ClientRect.right;
            temp.InflateRect(3, 6);
        }
        if (*item.szTip)
            InvalidateRect(&temp, false);
        else InvalidateRect(&item.ItemRect, false);
    }
}

HyperLinkControlItem* CHyperLinkControl::getItemByCommand(int command) {
    for (size_t i = 0; i < Items.GetCount(); i++) {
        if (Items[i].idCommand == command) {
            return &Items[i];  
        }
    }
    return nullptr;
}

int CHyperLinkControl::selectedItemIndex() const {
    return selectedItemIndex_;
}

BOOL CHyperLinkControl::OnSetCursor(CWindow/* wnd*/, UINT/* nHitTest*/, UINT/* message*/)
{
    //if(m_bHyperLinks)
    {
        bool SubItem = false;
        if(hoverItemIndex_ != -1)
            if(*Items[hoverItemIndex_].szTip == 0)
                SubItem = true;
        CursorHand = (hoverItemIndex_ !=-1)&&(m_bHyperLinks || SubItem);
        SetCursor(CursorHand? SetCursor(HandCursor) : LoadCursor(NULL,IDC_ARROW));
    }

    return TRUE;
}

LRESULT CHyperLinkControl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    for (size_t i = 0; i < Items.GetCount(); i++) {
        if (Items[i].hIcon) {
            DestroyIcon(Items[i].hIcon);
        }
    }
    return 0;
}

LRESULT CHyperLinkControl::OnGetObject(UINT, WPARAM wParam, LPARAM lParam) {
    if (lParam == OBJID_CLIENT) {
        if (acc_ == nullptr) {
            CHyperLinkControlAccessible::Create(this, &acc_);
        }
        return LresultFromObject(__uuidof(*acc_), wParam, acc_);
    } else {
        SetMsgHandled(FALSE);
        return 0;
    }
}

LRESULT CHyperLinkControl::OnGetDlgCode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    LRESULT retFlags = DLGC_WANTARROWS | DLGC_DEFPUSHBUTTON | DLGC_WANTTAB;
    if (wParam == VK_RETURN ) {
        if (selectedItemIndex_ != -1) {
            //NotifyParent(Selected);
            retFlags |= DLGC_DEFPUSHBUTTON;
        }
    } 
    return retFlags;
}

int CHyperLinkControl::ScaleX(int x) const 
{  
    return MulDiv(x, dpiX, 96); 
}
int CHyperLinkControl::ScaleY(int y) const { 
    return MulDiv(y, dpiY, 96);
}
