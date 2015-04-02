/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "atlheaders.h"
#include "HyperLinkControl.h"
#include "Func/Common.h"
#include "Gui/GuiTools.h"

// CHyperLinkControl
CHyperLinkControl::CHyperLinkControl()
{
	MouseSel = false;
	Track = false;
	BottomY = 1;
	SubItemRightY = -1;
	Selected = -1;
	CursorHand = false;
	m_bHyperLinks = true;
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
	BoldFont = MakeFontBold(NormalFont);
	UnderlineFont = MakeFontUnderLine(NormalFont);
	BoldUnderLineFont = MakeFontUnderLine(BoldFont);
	m_BkColor = BkColor;
	OpenThemeData();
}

int GetTextWidth(HDC dc, LPTSTR Text, HFONT Font)
{
	SIZE sz;
	HGDIOBJ  OldFont = SelectObject(dc, Font);
	GetTextExtentPoint32(dc, Text, lstrlen(Text), &sz);
	SelectObject(dc, OldFont);
	return sz.cx;
}

int CHyperLinkControl::AddString(LPTSTR szTitle,LPTSTR szTip,int idCommand,HICON hIcon,bool Visible,int Align,  bool LineBreak)
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
	CDC dc = GetDC();

	int TitleWidth = max(GetTextWidth(dc, szTitle, BoldFont),
		GetTextWidth(dc, szTip, NormalFont));

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
			if(Selected == i ) return 0;
			Items[i].Hover = true;
			SelectItem(i);

			return 0;
		}
	}

	SelectItem(-1); 

	return 0;
}

LRESULT CHyperLinkControl::OnMouseLeave(void)
{
	SelectItem(-1);
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
	if(Selected != -1)  return 0;

	if(hwndOldFocus == ::GetNextDlgTabItem(::GetParent(GetParent()), m_hWnd,	true) || hwndOldFocus == ::GetNextDlgTabItem(::GetParent(GetParent()), m_hWnd,	false)) 
		SelectItem(0);

	return 0;
}

LRESULT CHyperLinkControl::OnLButtonUp(UINT Flags, CPoint Pt)
{
	BOOL OutSide=false;
	RECT ClientRect;
	GetClientRect(&ClientRect);

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
			NotifyParent(i);
		}
	}

	return 0;
}

LRESULT CHyperLinkControl::OnKeyUp(TCHAR vk, UINT cRepeat, UINT flags)
{
	if (vk==VK_DOWN)
	{
		int NewSelect = (Selected+1)%Items.GetCount();
		if(!Items[NewSelect].Visible) 
			NewSelect = (NewSelect+1)%Items.GetCount();
		SelectItem( NewSelect);
	}

	else if (vk==VK_UP)
	{
		int NewSelect=Selected-1;
		if(NewSelect<0) NewSelect=Items.GetCount()-1;
		if(!Items[NewSelect].Visible) NewSelect--;


		SelectItem(NewSelect);
	}


	if (vk==VK_RETURN || vk==VK_SPACE)
	{
		if(Selected!=-1) 
			NotifyParent(Selected);
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

	HyperLinkControlItem item;
	CRect r(rc);
	CBrush br;
	//br.CreateSolidBrush(RGB(0,0,0));
	br.CreateSolidBrush(m_BkColor);
	dc.FillRect(r,br);

	dc.SetBkMode(TRANSPARENT);

	for(size_t i=0; i<Items.GetCount(); i++)
	{
		if(!Items[i].Visible) continue;
		item = Items[i];
		dc.SetTextColor(RGB(0,0,0));

		if(*(item.szTip)) // If we draw "big" item (with tip)
		{
			RECT TextRect = item.ItemRect;
			if(!m_bHyperLinks && Selected == i)
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
			if(Selected == i && m_bHyperLinks)
				dc.SelectFont(BoldUnderLineFont); else
				dc.SelectFont(BoldFont);

			int textY = item.ItemRect.top;
			if(*item.szTip == _T(' '))
			{
				SIZE TextDims;
				GetTextExtentPoint32(dc, item.szTitle, lstrlen(item.szTitle),	&TextDims);
				textY += (33-TextDims.cy)/2;

			}
		

			ExtTextOutW(dc.m_hDC, TextRect.left, textY, ETO_CLIPPED, &TextRect, item.szTitle, wcslen(item.szTitle), 0);
			if(Selected==i && m_bHyperLinks)
				dc.SelectFont(UnderlineFont); else 
				dc.SelectFont(GetFont());
			dc.SetTextColor(RGB(100,100,100));

			ExtTextOutW(dc.m_hDC, TextRect.left, item.ItemRect.top+18, ETO_CLIPPED, &TextRect, item.szTip, wcslen(item.szTip), 0);

			dc.SetBkMode(TRANSPARENT);
			if(m_bHyperLinks){
				dc.DrawIcon(item.ItemRect.left+1,item.ItemRect.top+2,item.hIcon);
			}
			else dc.DrawIcon(item.ItemRect.left+15,item.ItemRect.top+2,item.hIcon);
		} // End of drawing "big" item

		else 
		{
			if(Selected==i) // If item we draw is selected
				dc.SelectFont(UnderlineFont); 
			else
				dc.SelectFont(GetFont());

			dc.DrawIconEx(item.ItemRect.left,item.ItemRect.top+1,item.hIcon,16,16);
				
			dc.ExtTextOut(item.ItemRect.left+23, item.ItemRect.top+1, ETO_CLIPPED/*|ETO_OPAQUE/**/, &item.ItemRect, item.szTitle, wcslen(item.szTitle), 0);
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
	if(Index == Selected) return; 

	RECT ClientRect;
	GetClientRect(&ClientRect);

	if(Selected != -1) // Repainting previous selection
	{
		CRect temp = Items[Selected].ItemRect;
		if(!m_bHyperLinks)
		{
			temp.right= ClientRect.right;
			temp.InflateRect(3,6);
		}
		if(*Items[Selected].szTip)
			//temp.left+=40;
			InvalidateRect(&temp,false);
		else InvalidateRect(&Items[Selected].ItemRect,false);
	}

	Selected = Index;

	if(Selected!=-1)  //Repainting selected hyperlink
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
	}
}

BOOL CHyperLinkControl::OnSetCursor(CWindow/* wnd*/, UINT/* nHitTest*/, UINT/* message*/)
{
	//if(m_bHyperLinks)
	{
		bool SubItem = false;
		if(Selected != -1)
			if(*Items[Selected].szTip == 0)
				SubItem = true;
		CursorHand = (Selected!=-1)&&(m_bHyperLinks || SubItem);
		SetCursor(CursorHand? SetCursor(HandCursor) : LoadCursor(NULL,IDC_ARROW));
	}

	return TRUE;
}

int CHyperLinkControl::ScaleX(int x) 
{  
	return MulDiv(x, dpiX, 96); 
}
int CHyperLinkControl::ScaleY(int y) { 
	return MulDiv(y, dpiY, 96);
}
