/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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

#include "stdafx.h"
#include "ColorPicker.h"

// CColorPicker
CColorPicker::CColorPicker()
{
	Color = 0;
}

CColorPicker::~CColorPicker()
{
}

LRESULT CColorPicker::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true;
	
	PAINTSTRUCT ps;
	BeginPaint(&ps);
	
	RECT rc;
	GetClientRect(&rc);

	HDC hdc = ps.hdc;
	HBRUSH Background = CreateSolidBrush(Color);

	SelectObject(hdc, Background);
	Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));

	DeleteObject(Background);

	EndPaint(&ps);
	return 0;
}

LRESULT CColorPicker::OnEraseBkg(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	bHandled = true;
	return 0;
}
	

LRESULT CColorPicker::OnLButtonDown(UINT, CPoint Pt)
{
	ChooseColor();
	return 0;
}

LRESULT CColorPicker::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(wParam == VK_SPACE)
		ChooseColor();
	return 0;
}

BOOL CColorPicker::ChooseColor()
{
	CColorDialog ColorDialog(Color);
	if(ColorDialog.DoModal(m_hWnd) == IDOK)
	{
		Color = ColorDialog.GetColor();
		Invalidate();
		return TRUE;
	}
	return FALSE;
}
	