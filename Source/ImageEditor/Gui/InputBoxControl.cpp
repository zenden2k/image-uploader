/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
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
#include <Gui/GuiTools.h>
#include <Core/Logging.h>

namespace ImageEditor {
// CLogListBox
InputBoxControl::InputBoxControl() {

}

InputBoxControl::~InputBoxControl() {
	//Detach();
}

LRESULT InputBoxControl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled)
{
	bHandled = false;
	SetEventMask(ENM_CHANGE|ENM_REQUESTRESIZE|ENM_SELCHANGE );
	
	SetFont(GuiTools::MakeFontBigger(GuiTools::GetSystemDialogFont()));
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
	SetWindowPos(/*HWND_TOP*/0,x,y,w,h, 0);
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

void InputBoxControl::setTextColor(Gdiplus::Color color)
{
	CHARFORMAT cf;
	memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.crTextColor = color.ToCOLORREF();
	cf.dwMask = CFM_COLOR;
	//SetSel(0, -1);
	SendMessage(EM_SETCHARFORMAT, IsWindowVisible() ? SCF_SELECTION : SCF_ALL, (LPARAM) &cf);
	//SetSel(-1, 0);
}

void InputBoxControl::setFont(LOGFONT font,  DWORD changeMask)
{
	if ( !IsWindowVisible() ) {
		CFont f;
		f.CreateFontIndirect(&font);
		SetFont(f);
	} else {
		CHARFORMAT cf = GuiTools::LogFontToCharFormat(font);
		cf.dwMask = changeMask;
		SendMessage(EM_SETCHARFORMAT, IsWindowVisible() ? SCF_SELECTION : SCF_ALL, (LPARAM) &cf);
	}
}

void InputBoxControl::setRawText(const std::string& text)
{
	std::stringstream rawRtfText(text);
	EDITSTREAM es = {0};
	es.dwCookie = (DWORD_PTR) &rawRtfText;
	es.pfnCallback = &EditStreamInCallback; 
	SendMessage(EM_STREAMIN, SF_RTF, (LPARAM)&es);
}

std::string InputBoxControl::getRawText()
{
	std::stringstream rawRtfText;
	EDITSTREAM es = {0};
	es.dwCookie = (DWORD_PTR) &rawRtfText;
	es.pfnCallback = &EditStreamOutCallback; 
	int bytes = SendMessage(EM_STREAMOUT, SF_RTF, (LPARAM)&es);
	LOG(INFO) << "getRawText, bytes=" << bytes;
	return rawRtfText.str();
}

DWORD CALLBACK InputBoxControl::EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	std::stringstream* rtf = reinterpret_cast<std::stringstream*>(dwCookie);
	rtf->write((CHAR*)pbBuff, cb);
	*pcb = cb;
	return 0;
}

DWORD CALLBACK InputBoxControl::EditStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	std::stringstream* rtf = reinterpret_cast<std::stringstream*>(dwCookie);
	*pcb = rtf->readsome((CHAR*)pbBuff, cb);
	return 0;
}

LRESULT InputBoxControl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,BOOL& bHandled) {
	return 0;
}

LRESULT InputBoxControl::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	WPARAM vKey = wParam;

	if ( vKey == VK_ESCAPE ) {
		if ( onEditCanceled ) {
			onEditCanceled();
		}
		bHandled = true;
	} else if ( vKey == VK_RETURN && GetKeyState(VK_CONTROL) & 0x80 && onEditFinished ) {
		onEditFinished();
	}

	return 1;
}

LRESULT InputBoxControl::OnChange(UINT wNotifyCode,int, HWND)
{
	/*CRect formatRect;
	GetRect( &formatRect );
	//Logger::Write(_T("%d %d %d %d\r\n"), r );
	CRect clientRect;
	GetClientRect( &clientRect );
	FORMATRANGE fr;
	CDC dc = GetDC();
	ZeroMemory(&fr, sizeof(fr));
	fr.hdc = dc;
	fr.hdcTarget = dc;
	fr.rc.left =0;
	fr.rc.top = 0;
	fr.rc.right = 0;
	fr.rc.bottom = 0;
	fr.chrg.cpMax = -1;                    //Indicate character from to character to 
	fr.chrg.cpMin = 0;
	FormatRange(&fr, FALSE);
	CRect r = fr.rc;
	LOG(INFO) << "r="<< r.left<< " " <<  r.top << " " << r.right - r.left << " " << r.bottom - r.top;

	if (r.Width()  > clientRect.Width() || r.Height() > clientRect.Height() ) {
		SetWindowPos( 0, 0,0, r.Width() + clientRect.Width() - formatRect.Width() +5 , 
			r.Height() + clientRect.Height() - formatRect.Height()+5, SWP_NOMOVE );
	}*/

	if ( onTextChanged ) {
		onTextChanged(L"");
	}
	return 0;
}

LRESULT InputBoxControl::OnRequestResize(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	bHandled = true;
	//LOG(INFO) << "OnRequestResize";
	REQRESIZE * pReqResize = (REQRESIZE *) pnmh; 
	
	if ( onResized ) {
		SIZE sz = { pReqResize->rc.right - pReqResize->rc.left, pReqResize->rc.bottom - pReqResize->rc.top };
		//LOG(INFO) << sz.cx << " " << sz.cy;
		RECT windowRect;
		GetWindowRect(&windowRect);
		onResized(/*sz.cx*/windowRect.right - windowRect.left, sz.cy);
	}

	//*SetWindowPos(0, 0,0, pReqResize->rc.right - pReqResize->rc.left, pReqResize->rc.bottom - pReqResize->rc.top);
	return 0;
}

LRESULT InputBoxControl::OnSelChange(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	SELCHANGE* selChange = reinterpret_cast<SELCHANGE*>(pnmh);
	CHARFORMAT cf;

	GetSelectionCharFormat(cf);
	if ( onSelectionChanged ) {
		onSelectionChanged(selChange->chrg.cpMin, selChange->chrg.cpMax, GuiTools::CharFormatToLogFont(cf));
	}

	return 0;
}

}