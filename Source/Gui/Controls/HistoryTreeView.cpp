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

#include "HistoryTreeView.h"
#include "Func/Myutils.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/Common.h"
#include "Gui/GuiTools.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"

const int LLB_VertDivider = 10;
const int LLB_VertMargin = 5;

HFONT MakeFontBold(HFONT font);

HFONT MakeFontUnderLine(HFONT font);

// CHistoryTreeView
CHistoryTreeView::CHistoryTreeView()
{
	ErrorIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ERRORICON));
	WarningIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONWARNING));
	m_thumbWidth = 56;
}

CHistoryTreeView::~CHistoryTreeView()
{
	CWindow::Detach();
}

LRESULT CHistoryTreeView::OnDrawitem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
	if (!dis)
		return FALSE;

	HistoryTreeViewItem* item = (HistoryTreeViewItem*)dis->itemData;
	if (!item)
		return FALSE;

	CDCHandle dc = dis->hDC;

	if (dis->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
	{
		dc.SetBkColor(GetSysColor(COLOR_WINDOW));
		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
		CRect r(dis->rcItem);
		if (!(dis->itemState & ODS_SELECTED))
		{
			CBrush br;
			br.CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			dc.FillRect(r, br);
		}
		CRect rct;
		GetClientRect(&rct);

		if (dis->itemState & ODS_SELECTED )
		{
			CRect rd(dis->rcItem);
			GuiTools::FillRectGradient(dis->hDC, rd, 0xEAE2D9, 0xD3C1AF, false);
		}
/*		else if(dis->itemID != GetCount()-1) // If it isn't last item
      {
         CPen pen;
         pen.CreatePen(PS_SOLID, 1, RGB(190,190,190));
         SelectObject(dc.m_hDC, pen);
         dc.MoveTo(rct.left, r.bottom-1);
         dc.LineTo(rct.right, r.bottom-1);
      }*/

		SetBkMode(dc.m_hDC, TRANSPARENT);

		SIZE TimeLabelDimensions;
		SelectObject(dc.m_hDC, NormalFont);
		GetTextExtentPoint32(dc, item->Time, item->Time.GetLength(), &TimeLabelDimensions);

		// Writing error time

		ExtTextOutW(dc.m_hDC, rct.right - 5 - TimeLabelDimensions.cx, r.top + LLB_VertMargin, ETO_CLIPPED, r, item->Time,
		            item->Time.GetLength(), 0);
		// Writing error title
		SelectObject(dc.m_hDC, UnderlineFont);
		ExtTextOutW(dc.m_hDC, r.left + 56, r.top + LLB_VertMargin, ETO_CLIPPED, r, item->strTitle, wcslen(
		               item->strTitle), 0);

		// Writing some info
		SelectObject(dc.m_hDC, NormalFont);
		RECT ItemRect = {r.left + 56, r.top + LLB_VertMargin + LLB_VertDivider + item->TitleHeight,
			              r.right - 10, r.bottom - LLB_VertMargin};
		dc.DrawText(item->Info, item->Info.GetLength(), &ItemRect, 0);

		// Writing error text with bold (explication of error)
		SelectObject(dc.m_hDC, BoldFont);
		RECT TextRect =
		{r.left + 56, LLB_VertMargin + r.top + item->TitleHeight + LLB_VertDivider +
		((item->Info.GetLength()) ? (item->InfoHeight + LLB_VertDivider) : 0), r.right - 10, r.bottom - LLB_VertMargin};
		dc.DrawText(item->strText,  wcslen(item->strText), &TextRect, 0);

/*		if(item->Type == logError)
         dc.DrawIcon(12,r.top+8,ErrorIcon);
      else if(item->Type == logWarning)
         dc.DrawIcon(12,r.top+8,WarningIcon);*/
	}
	bHandled = true;
	return 0;
}

LRESULT CHistoryTreeView::OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;

	HistoryTreeViewItem* item = (HistoryTreeViewItem*)lpmis->itemData;
	if (!item)
		return 0;
	HDC dc = GetDC();
	SelectObject(dc, NormalFont);

	RECT ClientRect;
	GetClientRect(&ClientRect);

	int ItemWidth = ClientRect.right - ClientRect.left - 50;

	RECT Dimensions = {0, 0, ItemWidth, 0};

	DrawText(dc, item->strTitle, lstrlen(item->strTitle), &Dimensions,   DT_CALCRECT);
	item->TitleHeight = Dimensions.bottom - Dimensions.top;

	// —читаем размеры подзаголовка
	Dimensions.bottom = 0;
	DrawText(dc, item->Info, item->Info.GetLength(), &Dimensions,  DT_CALCRECT);
	item->InfoHeight = Dimensions.bottom - Dimensions.top;

	SelectObject(dc, BoldFont);
	// —читаем размеры основного текста
	Dimensions.bottom = 0;
	DrawText(dc, item->strText, lstrlen(item->strText), &Dimensions,  DT_CALCRECT);
	item->TextHeight = Dimensions.bottom - Dimensions.top;
	SelectObject(dc, NormalFont);
	CString str;
	lpmis->itemWidth = ItemWidth;
	lpmis->itemHeight = LLB_VertMargin + item->TitleHeight + LLB_VertDivider + item->TextHeight +
	   (item->InfoHeight ? (LLB_VertDivider + item->InfoHeight) : 0) + LLB_VertMargin + 2;
	lpmis->itemHeight = max(lpmis->itemHeight, 35);
	ReleaseDC(dc);
	return 0;
}

int CHistoryTreeView::AddString( LPCTSTR strTitle, LPCTSTR strText, LPCTSTR szInfo)
{
	HistoryTreeViewItem* item = new  HistoryTreeViewItem;
	// item->Type = Type;

	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString Data;
	Data.Format(_T("%02d:%02d:%02d"), (int)st.wHour, (int)st.wMinute, (int)st.wSecond);

	/*item->strText = trim(strText);
	item->strTitle = trim(strTitle);
	item->Info = trim(szInfo);*/
	item->Time = Data;

	SetRedraw(FALSE);
//	int nPos = CListBox::AddString((LPCTSTR)item);

	// if(nPos < 0) return -1;

/*   SetItemDataPtr(nPos, item);
   SetTopIndex(nPos-1);
   SetCurSel(nPos);*/
	SetRedraw(TRUE);
	return 0;
}

LRESULT CHistoryTreeView::OnKillFocus(HWND hwndNewFocus)
{
//	SetCurSel(-1);
	return 0;
}

BOOL CHistoryTreeView::SubclassWindow(HWND hWnd)
{
	BOOL Result = CWindowImpl<CHistoryTreeView, CTreeViewCtrl, CControlWinTraits>::SubclassWindow(hWnd);
	Init();
	return Result;
}

void CHistoryTreeView::Init()
{
	NormalFont = GetFont();
	UnderlineFont =  MakeFontUnderLine(NormalFont);
	BoldFont = MakeFontBold(NormalFont);
	SetItemHeight(1);
}

LRESULT CHistoryTreeView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Clear();
	return 0;
}

void CHistoryTreeView::Clear()
{
	/*SetRedraw(false);

	int n = GetCount();
	for(int i= 0; i<n; i++)
	{
	      HistoryTreeViewItem * item =(HistoryTreeViewItem *)GetItemDataPtr(i);
	      delete item;
	}
	ResetContent();
	SetRedraw(true);*/
}

void CHistoryTreeView::addSubEntry(HTREEITEM res, HistoryItem it)
{
	HTREEITEM item = InsertItem(Utf8ToWCstring(IuCoreUtils:: timeStampToString(
	                                              it.timeStamp) + " " + it.localFilePath), 1, 1, res, TVI_SORT);
	TVITEMEX p;
	p.mask = TVIF_INTEGRAL | TVIF_PARAM;
	p.hItem = item;
	p.iIntegral = m_thumbWidth + 4;
	HistoryItem* it2 = new HistoryItem(it);
	p.lParam = reinterpret_cast<LPARAM>(it2);
	SetItem(&p);
}

DWORD CHistoryTreeView::OnPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
{
	return CDRF_NOTIFYITEMDRAW /*|CDRF_NOTIFYSUBITEMDRAW*/;
}

DWORD CHistoryTreeView::OnItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW lpNMCustomDraw)
{
	NMTVCUSTOMDRAW* pLVCD = reinterpret_cast<NMTVCUSTOMDRAW*>(lpNMCustomDraw);
	HTREEITEM item  = (HTREEITEM) pLVCD->nmcd.dwItemSpec;
	HDC dc = pLVCD->nmcd.hdc;
	// Beep(1000, 250);
	if ( TreeView_GetParent(m_hWnd, item ) != 0)
	{
		// return CDRF_DODEFAULT;
		DrawSubItem(item,  dc, pLVCD->nmcd.uItemState, pLVCD->nmcd.rc, 0);

		return CDRF_SKIPDEFAULT;
	}
	else
	{
		// return CDRF_DODEFAULT;
		DrawItem(item,  dc, pLVCD->nmcd.uItemState, pLVCD->nmcd.rc, 0);
		return CDRF_SKIPDEFAULT;
	}
}

DWORD CHistoryTreeView::OnSubItemPrePaint(int /*idCtrl*/, LPNMCUSTOMDRAW /*lpNMCustomDraw*/)
{
	return CDRF_DODEFAULT;
}

HistoryItem* CHistoryTreeView::getItemData(HTREEITEM res)
{
	return reinterpret_cast<HistoryItem*>(GetItemData(res));
}

HICON CHistoryTreeView::getIconForExtension(const CString& ext)
{
	if (m_fileIconCache[ext] != 0)
	{
		return m_fileIconCache[ext];
	}
	HICON res = GetAssociatedIcon(ext, false);
	if (!res)
		return 0;
	m_fileIconCache[ext] = res;
	return res;
}

HTREEITEM CHistoryTreeView::addEntry(CHistorySession* session, const CString& text)
{
	TVINSERTSTRUCT is;
	memset(&is, 0, sizeof(is));
	is.itemex.mask = TVIF_PARAM | TVIF_TEXT | TVIF_INTEGRAL;
	TCHAR buf[500];
	lstrcpyn(buf, text, sizeof(buf) / sizeof(TCHAR));
	is.itemex.pszText = buf;
	is.itemex.lParam = reinterpret_cast<LPARAM>(session);
	// SetItem(&p); // If we won't do this we will get access violation
	is.itemex.iIntegral = 30;
	HTREEITEM item = InsertItem(&is);
	return item;
}

void CHistoryTreeView::DrawItem(HTREEITEM item, HDC hdc, DWORD itemState, RECT invRC, int* outHeight)
{
	int curY = 0;
	bool draw = !outHeight;

	bool isSelected = (itemState & CDIS_SELECTED) || (itemState & CDIS_FOCUS);
	CRect clientRect;
	GetClientRect(clientRect);
	// HistoryItem * it2 = new HistoryItem(it);
	CHistorySession* ses = reinterpret_cast<CHistorySession*>(GetItemData(item));
	std::string label = "[" + IuCoreUtils::timeStampToString(ses->timeStamp()) + "]";
	std::string serverName = ses->serverName();
	if (serverName.empty())
		serverName = "uknown server";
	std::string lowText =
	   serverName + " (" + IuCoreUtils::toString(ses->entriesCount()) + " files)";

	CString text = Utf8ToWCstring(label);

	CRect rc;
	CRect calcRect;

	GetItemRect(item, &rc, true);
	CDC dc (hdc);
	CBrush br;

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(0, 0, 0));
	CBrush backgroundBrush;

	// GetItemRect(item, &fullRect, false);
	DWORD color = RGB(255, 255, 255);
	if (itemState & CDIS_SELECTED)
		color = 0x9fd5ff;
	backgroundBrush.CreateSolidBrush(color);
	if (draw)
		dc.FillRect(&invRC, backgroundBrush);
	DrawText(dc.m_hDC, text, text.GetLength(), &calcRect, DT_CALCRECT);
	calcRect.OffsetRect(rc.left, rc.top);
	// rc.top += 1;
	curY += 1;

	if (draw)
	{
		CRect dateRect = rc;
		dateRect.OffsetRect(400, 0);
		dc.SetTextColor(0x909090);
		DrawText(dc.m_hDC, text, text.GetLength(), &dateRect, DT_LEFT);
		dc.SetTextColor(0);
	}
	// curY+= calcRect.Height();

	int curX = 0;

	RECT gradientLineRect;
	GetItemRect(item, &gradientLineRect, false);
	gradientLineRect.bottom--;
	gradientLineRect.top = gradientLineRect.bottom;
	if (draw)
		GuiTools::FillRectGradient(hdc, gradientLineRect, 0xc8c8c8, 0xFFFFFF, true);
	// rc.top = curY+3;
	calcRect = rc;
	DrawText(dc.m_hDC, Utf8ToWCstring(lowText), lowText.length(), &calcRect, DT_CALCRECT);
	// calcRect.top = curY+3;
	// calcRect.left+=30;

	if (draw)
	{
		bool isItemExpanded = (GetItemState(item, TVIS_EXPANDED) & TVIS_EXPANDED) != 0;
		CRect plusIconRect;
		SIZE plusIconSize = {9, 9};
		HTHEME theme = OpenThemeData(_T("treeview"));
		if (!theme)
		{
			GetThemePartSize(dc.m_hDC, TVP_GLYPH, isItemExpanded ? GLPS_OPENED : GLPS_CLOSED, 0, TS_DRAW, &plusIconSize);
		}

		int iconOffsetX = 3;
		int iconOffsetY = (rc.Height() - plusIconSize.cy) / 2;
		plusIconRect.left = rc.left + iconOffsetX;
		plusIconRect.top = rc.top + iconOffsetY;
		plusIconRect.right = plusIconRect.left + plusIconSize.cx;
		plusIconRect.bottom = plusIconRect.top + plusIconSize.cy;
		curX  += iconOffsetX + plusIconSize.cx;
		if (theme)
		{
			DrawThemeBackground( dc.m_hDC, TVP_GLYPH, isItemExpanded ? GLPS_OPENED : GLPS_CLOSED, &plusIconRect);  // закрыта€ папка
			CloseThemeData();
		}
		else
		{
			CRect FrameRect(plusIconRect);
			dc.FillSolidRect(FrameRect, 0x808080);
			FrameRect.DeflateRect(1, 1, 1, 1);
			dc.FillSolidRect(FrameRect, 0xFFFFFF);

			CRect MinusRect(2, 4, 7, 5);
			MinusRect.OffsetRect(plusIconRect.TopLeft());
			dc.FillSolidRect(MinusRect, 0x000000);

			if (!isItemExpanded)
			{
				CRect PlusRect(4, 2, 5, 7);
				PlusRect.OffsetRect(plusIconRect.TopLeft());
				dc.FillSolidRect(PlusRect, 0x000000);
			}
		}
	}

	CRect drawRect;
	int serverIconOffsetY = (rc.Height() - 1 - 16) / 2;
	drawRect.top = rc.top + serverIconOffsetY;
	drawRect.bottom = drawRect.top + calcRect.Height();
	drawRect.left = rc.left + curX + 4;
	drawRect.right = drawRect.left + calcRect.Width();
	HICON ico = getIconForServer(Utf8ToWCstring(ses->serverName()));
	if (ico)
	{
		dc.DrawIconEx(drawRect.left, drawRect.top, ico, 16, 16);
	}
	drawRect.OffsetRect(16 + 3, 0);
	;
	if (draw)
		DrawText(dc.m_hDC,  Utf8ToWCstring(lowText), lowText.length(), &drawRect, DT_LEFT | DT_VCENTER);

	curY += max(calcRect.Height(), /* server icon height */ 16);
	dc.Detach();
	curY += 3;
	if (outHeight)
		*outHeight = curY;
}

int CHistoryTreeView::CalcItemHeight(HTREEITEM item)
{
	int res = 0;
	HDC dc =  GetDC();
	RECT rc;
	GetItemRect(item, &rc, false);
	DrawItem(item, dc, 0, rc,  &res);
	// dc.Detach();
	ReleaseDC(dc);
	return res;
}

void CHistoryTreeView::DrawSubItem(HTREEITEM item, HDC hdc, DWORD itemState, RECT invRC, int* outHeight)
{
	RECT rc;
	GetItemRect(item, &rc, true);
	CDC dc (hdc);
	CBrush br;
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(0, 0, 0));

	DWORD color = RGB(255, 255, 255);
	int indent = static_cast<int>(GetIndent() * 1.5);
	if (itemState & CDIS_SELECTED)
		color = 0x9fd5ff;
	RECT fullRect;
	GetItemRect(item, &fullRect, false);

	// fullRect.left += indent;

	CBrush backgroundBrush;
	backgroundBrush.CreateSolidBrush(color);
	dc.FillRect(&rc, backgroundBrush);

	// RECT itemHitRect;
	br.CreateSolidBrush(0x9fd5ff);
	// rc.left += indent;

	RECT thumbRect;
	thumbRect.left = rc.left + 10;
	thumbRect.top = rc.top + 2;
	thumbRect.bottom = thumbRect.top + m_thumbWidth;
	thumbRect.right = thumbRect.left + m_thumbWidth;

	/*	RECT thumbRect = {fullRect.left, fullRect.top+1,
	      fullRect.left+m_thumbWidth, fullRect.top + m_thumbWidth -1 };*/
	// thumbRect.right  = rc.left + m_thumbWidth;
	// GetItemPartRect(item, &itemHitRect, 1);
	dc.FrameRect(&thumbRect, br);
	HistoryItem* it2 = getItemData(item);
	std::string fileName = IuCoreUtils::ExtractFileName(it2->localFilePath);
	CString ext = Utf8ToWstring(IuCoreUtils::ExtractFileExt(fileName)).c_str();

	HICON ico = getIconForExtension(Utf8ToWCstring(it2->localFilePath));
	CString text = Utf8ToWstring(fileName).c_str();
	int iconWidth = 32;
	int iconHeight = 32;
	if (ico != 0)
		dc.DrawIcon(thumbRect.left + 1 +
		            (m_thumbWidth - iconWidth) / 2, thumbRect.top + 1 + (m_thumbWidth - iconHeight) / 2,
		            ico);

	//	CRect textRect
	CRect calcRect = invRC;
	calcRect.left = thumbRect.right + 5;
	DrawText(dc.m_hDC, text, text.GetLength(), &calcRect, DT_CALCRECT);
	// ExtTextOutW(dc.m_hDC,  thumbRect.right  + 4, rc.top, ETO_CLIPPED, &rc, text, text.GetLength(), 0);
	int filenameHeight = calcRect.Height();
	// ShowVar(filenameHeight);
	DrawText(dc.m_hDC, text, text.GetLength(), &calcRect, DT_LEFT);

	CRect urlRect = invRC;
	urlRect.left = calcRect.left;
	urlRect.top += filenameHeight + 3;

	CString url =  Utf8ToWCstring(it2->directUrl.length() ? it2->directUrl : it2->viewUrl);
	dc.SetTextColor(0xa6a6a6);

	DrawText(dc.m_hDC, url, url.GetLength(), &urlRect, DT_LEFT);

	dc.Detach();
}

HICON CHistoryTreeView::getIconForServer(const CString& serverName)
{
	HICON ico = 0;

	if (m_serverIconCache[serverName] != 0)
		return m_serverIconCache[serverName];
	else

		ico = (HICON)LoadImage(0, IuCommonFunctions::GetDataFolder() + _T("Favicons\\") + serverName + _T(
		                          ".ico"), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	m_serverIconCache[serverName] = ico;
	return ico;
}

LRESULT CHistoryTreeView::OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = LOWORD(lParam);  // horizontal position of cursor
	int yPos = HIWORD(lParam);
	UINT flags = 0;
	xPos = 20;
	POINT pt = {xPos, yPos};
	HTREEITEM item = HitTest(pt, &flags);

	if (flags & TVHT_ONITEM  )
	{
		// GetItemState(item,TVE_TOGGLE);
		Expand(item, TVE_TOGGLE);
	}
	bHandled = true;
	return 0;
}

LRESULT CHistoryTreeView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int xPos = LOWORD(lParam);  // horizontal position of cursor
	int yPos = HIWORD(lParam);
	UINT flags = 0;
	if (xPos > 50)
		xPos = 50;
	POINT pt = {xPos, yPos};

	HTREEITEM item = HitTest(pt, &flags);
	// Beep(1000,300);
	if ((flags & TVHT_ONITEM) && xPos < 16  )
	{
		Expand(item, TVE_TOGGLE);
	}
	else if (flags & TVHT_ONITEM  && uMsg == WM_RBUTTONDOWN)
	{
		SelectItem(item);
		bHandled = true;
		return 0;
	}
	bHandled = false;
	return 0;
}

LRESULT CHistoryTreeView::ReflectContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	/*int xPos = GET_X_LPARAM(lParam);
	int  yPos = GET_Y_LPARAM(lParam);
	return 0;
	//ShowVar(yPos);
	if(xPos >50) xPos = 50;
	UINT flags = 0;
	POINT pt = {xPos, yPos};
	HTREEITEM item = HitTest(pt, &flags);
	if((flags & TVHT_ONITEM))
	{
	   SelectItem(item);
	}
	bHandled = true;*/
	return 0;
}

bool CHistoryTreeView::IsItemAtPos(int x, int y, bool& isRoot)
{
	if (x > 50)
		x = 50;
	UINT flags = 0;
	POINT pt = {x, y};
	HTREEITEM item = HitTest(pt, &flags);
	bool result = (flags & TVHT_ONITEM) != 0;
	if (result)
	{
		isRoot = TreeView_GetParent(m_hWnd, item ) == 0;
	}
	return result;
}
