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

#include "atlheaders.h"
#include "LogListBox.h"
#include "Func/Myutils.h"
#include "Gui/GuiTools.h"

const int LLB_VertDivider = 10;
const int LLB_VertMargin = 5;

HFONT MakeFontBold(HFONT font);

HFONT MakeFontUnderLine(HFONT font);

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
			ZGuiTools::FillRectGradient(dis->hDC,rd,0xEAE2D9, 0xD3C1AF, false);
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
	if(!item) return 0;
	HDC dc = GetDC();
	SelectObject(dc, NormalFont);


	RECT ClientRect;
	GetClientRect(&ClientRect);

	int ItemWidth = ClientRect.right - ClientRect.left - 50;
	

	RECT Dimensions={0, 0, ItemWidth, 0};

	
	DrawText(dc, item->strTitle, lstrlen(item->strTitle), &Dimensions,	DT_CALCRECT);
	item->TitleHeight = Dimensions.bottom - Dimensions.top;
	
	// ������� ������� ������������
	Dimensions.bottom = 0;
	DrawText(dc, item->Info, item->Info.GetLength(), &Dimensions,	DT_CALCRECT);
	item->InfoHeight = Dimensions.bottom - Dimensions.top;

	SelectObject(dc, BoldFont);
	// ������� ������� ��������� ������
	Dimensions.bottom = 0;
	DrawText(dc, item->strText, lstrlen(item->strText), &Dimensions,	DT_CALCRECT);
	item->TextHeight = Dimensions.bottom - Dimensions.top;
	SelectObject(dc, NormalFont);
	CString str;
	lpmis->itemWidth = ItemWidth;
	lpmis->itemHeight = LLB_VertMargin + item->TitleHeight + LLB_VertDivider + item->TextHeight + (item->InfoHeight?(LLB_VertDivider + item->InfoHeight):0) + LLB_VertMargin+2;
	lpmis->itemHeight = max(lpmis->itemHeight, 35);
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

int CLogListBox::AddString(LogMsgType Type, LPCTSTR strTitle, LPCTSTR strText, LPCTSTR szInfo)
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
	UnderlineFont =  MakeFontUnderLine(NormalFont);
	BoldFont = MakeFontBold(NormalFont);
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

