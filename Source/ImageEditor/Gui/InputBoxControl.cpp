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
#include "../Logger.h"
namespace ImageEditor {
// CLogListBox
InputBoxControl::InputBoxControl() {

}

InputBoxControl::~InputBoxControl() {
	Detach();
}

LRESULT InputBoxControl::OnKillFocus(HWND hwndNewFocus) {
	ShowWindow( SW_HIDE ); 
	return 0;
}

BOOL  InputBoxControl::SubclassWindow(HWND hWnd) {
	BOOL Result = Base::SubclassWindow(hWnd);
	return Result;
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
	Logger::Write(_T("%d %d %d %d\r\n"), r );
	CRect clientRect;
	GetClientRect( &clientRect );
	DWORD margins = GetMargins();
	if (r.Width()  > clientRect.Width() || r.Height() > clientRect.Height() ) {
		SetWindowPos( 0, 0, 0, r.Width() + clientRect.Width() - formatRect.Width() +5 , 
			r.Height() + clientRect.Height() - formatRect.Height()+5, SWP_NOMOVE );
	}
	return 1;
}

}