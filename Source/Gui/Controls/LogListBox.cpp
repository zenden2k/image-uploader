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

#include "atlheaders.h"
#include "LogListBox.h"
#include "Func/Myutils.h"
#include "Gui/GuiTools.h"
#include "Core/Logging.h"
#include <algorithm>

const int LLB_VertDivider = 10;
const int LLB_VertMargin = 5;


// CLogListBox
CLogListBox::CLogListBox()
{
    ErrorIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ERRORICON));
    WarningIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONWARNING));
}

CLogListBox::~CLogListBox()
{
    Detach();
}

LRESULT CLogListBox::OnDrawitem(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
    LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
    if(!dis) return FALSE;
      
    LogListBoxItem * item = (LogListBoxItem *)dis->itemData;
    if(!item) return FALSE;
    
   CDCHandle dc = dis->hDC;

   if(dis->itemAction & (ODA_DRAWENTIRE|ODA_SELECT))
   {
        dc.SetBkColor(GetSysColor(COLOR_WINDOW));
        dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
      CRect r(dis->rcItem);
        if(!(dis->itemState & ODS_SELECTED ))
        {
            CBrush br;
            br.CreateSolidBrush(GetSysColor(COLOR_WINDOW));
            dc.FillRect(r,br);
        }
        CRect rct;
      GetClientRect(&rct);

        if(dis->itemState & ODS_SELECTED )
        {
            CRect rd(dis->rcItem);
            GuiTools::FillRectGradient(dis->hDC,rd,0xEAE2D9, 0xD3C1AF, false);
        }
        else if(dis->itemID != GetCount()-1) // If it isn't last item
        {
            CPen pen;
            pen.CreatePen(PS_SOLID, 1, RGB(190,190,190));
            SelectObject(dc.m_hDC, pen);
            dc.MoveTo(rct.left, r.bottom-1);
            dc.LineTo(rct.right, r.bottom-1);
        }
              
        SetBkMode(dc.m_hDC,TRANSPARENT);

        SIZE TimeLabelDimensions;
        SelectObject(dc.m_hDC, NormalFont);
        GetTextExtentPoint32(dc, item->Time, item->Time.GetLength(), &TimeLabelDimensions);
        
        // Writing error time
        
        ExtTextOutW(dc.m_hDC, rct.right-5-TimeLabelDimensions.cx, r.top + LLB_VertMargin, ETO_CLIPPED, r, item->Time, item->Time.GetLength(), 0);
        // Writing error title
        SelectObject(dc.m_hDC, UnderlineFont);
        ExtTextOutW(dc.m_hDC, r.left+56, r.top + LLB_VertMargin, ETO_CLIPPED, r, item->strTitle, wcslen(item->strTitle), 0);
        
        // Writing some info
        SelectObject(dc.m_hDC, NormalFont);
        RECT ItemRect={r.left+56, r.top + LLB_VertMargin + LLB_VertDivider + item->TitleHeight, 
                            r.right - 10, r.bottom-LLB_VertMargin};
        dc.DrawText(item->Info, item->Info.GetLength() , &ItemRect, DT_NOPREFIX);
            
        // Writing error text with bold (explication of error)
        SelectObject(dc.m_hDC, BoldFont);
        RECT TextRect = {r.left+56, LLB_VertMargin +r.top+ item->TitleHeight+LLB_VertDivider+((item->Info.GetLength())?(item->InfoHeight+LLB_VertDivider):0), r.right - 10, r.bottom-LLB_VertMargin};
        dc.DrawText(item->strText,  wcslen(item->strText), &TextRect, DT_NOPREFIX);

        if(item->Type == logError)
            dc.DrawIcon(12,r.top+8,ErrorIcon);
        else if(item->Type == logWarning)
            dc.DrawIcon(12,r.top+8,WarningIcon);
    }
  
    bHandled = true;
    return 0;
}

LRESULT CLogListBox::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{

    LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;
    
    LogListBoxItem * item=(LogListBoxItem *)lpmis->itemData;
    if(!item) {
        return 0;
    }
    HDC dc = GetDC();
    //float dpiScaleX_ = GetDeviceCaps(dc, LOGPIXELSX) / 96.0f;
    float dpiScaleY_ = GetDeviceCaps(dc, LOGPIXELSY) / 96.0f;
    SelectObject(dc, NormalFont);


    RECT ClientRect;
    GetClientRect(&ClientRect);

    int ItemWidth = ClientRect.right - ClientRect.left - 50;
    

    RECT Dimensions={0, 0, ItemWidth, 0};

    
    DrawText(dc, item->strTitle, lstrlen(item->strTitle), &Dimensions,    DT_CALCRECT);
    item->TitleHeight = Dimensions.bottom - Dimensions.top;
    
    // —читаем размеры подзаголовка
    Dimensions.bottom = 0;
    DrawText(dc, item->Info, item->Info.GetLength(), &Dimensions,    DT_CALCRECT);
    item->InfoHeight = Dimensions.bottom - Dimensions.top;

    SelectObject(dc, BoldFont);
    // —читаем размеры основного текста
    Dimensions.bottom = 0;
    DrawText(dc, item->strText, item->strText.GetLength(), &Dimensions,    DT_CALCRECT);
    item->TextHeight = Dimensions.bottom - Dimensions.top;
    SelectObject(dc, NormalFont);
    CString str;
    lpmis->itemWidth = ItemWidth;
    lpmis->itemHeight = LLB_VertMargin + item->TitleHeight + LLB_VertDivider + item->TextHeight + (item->InfoHeight?(LLB_VertDivider + item->InfoHeight):0) + LLB_VertMargin+2;
    lpmis->itemHeight = std::max(lpmis->itemHeight, static_cast<UINT>(dpiScaleY_ * 70) );
    lpmis->itemHeight = std::min(254u , lpmis->itemHeight);

    ReleaseDC(dc);
    return 0;
}

CString trim(const CString& Str)
{
    CString Result = Str;
    if(!Result.IsEmpty())
        for(int i = Result.GetLength()-1;  i>=0;i--)
        {
            if(Result[i]==_T('\n'))
            Result.Delete(i);
            else break;
        }
    return Result;
}

int CLogListBox::AddString(LogMsgType Type, const CString& strTitle, const CString& strText, const CString& szInfo)
{
    LogListBoxItem * item = new  LogListBoxItem;
    item->Type = Type;

    SYSTEMTIME st;
    ::GetLocalTime(&st);
    CString Data;
    Data.Format(_T("%02d:%02d:%02d"), (int)st.wHour, (int)st.wMinute, (int)st.wSecond);
    
    item->strText = trim(strText);
    item->strTitle = trim(strTitle);
    item->Info = trim(szInfo);
    item->Time = Data;

    SetRedraw(FALSE);
    int nPos = CListBox::AddString((LPCTSTR)item);

   if(nPos < 0) return -1;

   SetItemDataPtr(nPos, item);
    SetTopIndex(nPos-1);
    SetCurSel(nPos);
    SetRedraw(TRUE);
    return nPos;
}

LRESULT CLogListBox::OnKillFocus(HWND hwndNewFocus)
{
    SetCurSel(-1);
    return 0;
}

BOOL  CLogListBox::SubclassWindow(HWND hWnd)
{
    BOOL Result = CWindowImpl<CLogListBox, CListBox,CControlWinTraits>::SubclassWindow(hWnd);
    Init();
    return Result;
}

void CLogListBox::Init()
{
    NormalFont = GetFont();
    UnderlineFont =  GuiTools::MakeFontUnderLine(NormalFont);
    BoldFont = GuiTools::MakeFontBold(NormalFont);
}

LRESULT CLogListBox::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
    Clear();
    return 0;
}

void CLogListBox::Clear()
{
    SetRedraw(false);

    int n = GetCount();
    for(int i= 0; i<n; i++)
    {
            LogListBoxItem * item =(LogListBoxItem *)GetItemDataPtr(i);
            delete item;
    }
    ResetContent();
    SetRedraw(true);
}

LogListBoxItem* CLogListBox::getItemFromIndex(int index) {
    return reinterpret_cast<LogListBoxItem *>( GetItemDataPtr(index) );
}
