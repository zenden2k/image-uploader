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

#include "InputBoxControl.h"
#include <Core/Images/Utils.h>

namespace ImageEditor {
// CLogListBox
InputBoxControl::InputBoxControl() {

}

InputBoxControl::~InputBoxControl() {
	DestroyWindow();
	//Detach();
}

LRESULT InputBoxControl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
	bHandled = false;
	SetEventMask(ENM_CHANGE);
	//SetWindowLong(GWL_ID, (LONG)m_hWnd);
	return 0;
}

LRESULT InputBoxControl::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
	bHandled = true;
	return 1;
}

LRESULT InputBoxControl::OnKillFocus(HWND hwndNewFocus) {
	//ShowWindow( SW_HIDE ); 
	return 0;
}

BOOL  InputBoxControl::SubclassWindow(HWND hWnd) {
	BOOL Result = Base::SubclassWindow(hWnd);

	return Result;
}

void InputBoxControl::show(bool s)
{
	ShowWindow(s? SW_SHOW: SW_HIDE);
}

void InputBoxControl::resize(int x, int y, int w,int h, std::vector<MovableElement::Grip> grips)
{
	SetWindowPos(HWND_TOP,x,y,w,h, 0);
	/*CRgn rgn;
	rgn.CreateRectRgn(0,0,w,h);
	for ( int i = 0; i < grips.size(); i++ ) {
		CRgn gripRgn;
		int gripX = grips[i].pt.x-x;
		int gripY = grips[i].pt.y-y;
		gripRgn.CreateRectRgn(gripX, gripY,gripX + MovableElement::kGripSize, gripY + MovableElement::kGripSize);
		rgn.CombineRgn(gripRgn, RGN_DIFF);
	}
	SetWindowRgn(rgn, true);
	rgn.Detach();*/
}

void InputBoxControl::render(Gdiplus::Graphics* graphics, Gdiplus::Bitmap* background, Gdiplus::Rect layoutArea)
{
	PrintRichEdit(m_hWnd, graphics, background, layoutArea);
}

bool InputBoxControl::isVisible()
{
	return IsWindowVisible();
}

void InputBoxControl::invalidate()
{
	Invalidate(false);
}

LRESULT InputBoxControl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled) {
	return 0;
}

LRESULT InputBoxControl::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WPARAM vKey = wParam;

	if ( vKey == VK_ESCAPE ) {
		ShowWindow( SW_HIDE );
		bHandled = true;
	}


	CRect r;
	TCHAR buf[512];
	GetWindowText( buf, sizeof(buf) /sizeof (*buf) );
	HDC hDC = GetDC();
	DrawText(hDC, buf, lstrlen( buf ) , &r, DT_CALCRECT);

	CRect formatRect;
	GetRect( &formatRect );
	//Logger::Write(_T("%d %d %d %d\r\n"), r );
	CRect clientRect;
	GetClientRect( &clientRect );
	/*DWORD margins = GetMargins();
	if (r.Width()  > clientRect.Width() || r.Height() > clientRect.Height() ) {
		SetWindowPos( 0, 0, 0, r.Width() + clientRect.Width() - formatRect.Width() +5 , 
			r.Height() + clientRect.Height() - formatRect.Height()+5, SWP_NOMOVE );
	}*/
	return 1;
}

LRESULT InputBoxControl::OnChange(UINT wNotifyCode,int, HWND)
{
	if ( onTextChanged ) {
		onTextChanged(L"");
	}
	return 0;
}

}