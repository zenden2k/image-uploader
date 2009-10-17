/*
    Image Uploader - application for uploading images/files to Internet
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
#include "RegionSelect.h"

CRegionSelect RegionSelect;

// CRegionSelect
CRegionSelect::CRegionSelect()
{
	Down = false;
	End.x = -1;
	End.y = -1;
	pen = CreatePen(PS_SOLID, 2, 0); //Solid black line, width = 2 pixels
	CrossCursor = LoadCursor(NULL,IDC_CROSS);
}

CRegionSelect::~CRegionSelect()
{ 
	if(pen)	DeleteObject(pen);
	if(CrossCursor)  DeleteObject(CrossCursor);
}

LRESULT CRegionSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;  // Let the system set the focus
}

LRESULT CRegionSelect::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Hide(false);
	return 0;
}

LRESULT CRegionSelect::OnKillFocus(HWND hwndNewFocus)
{
	Hide(false);
	return 0;
}

LRESULT CRegionSelect::OnActivate(UINT state, BOOL fMinimized, HWND hwndActDeact)
{
	return 0;
}

LRESULT CRegionSelect::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	BeginPaint(&ps);

	HDC dc = ps.hdc;
	if(Down) return 0;
	RECT rc;
	GetClientRect(&rc);

	BitBlt(dc,0,0,1280,1024,memDC2,0,0,SRCCOPY);
	EndPaint(&ps);

	return 0;
}

void CRegionSelect::ShowW(HWND Parent,HBITMAP bmp,int w,int h)
{
	Down = false;
	End.x = -1;

	End.y = -1;

	bm = bmp;
	memDC2 = ::CreateCompatibleDC(GetDC());

	// Создание битмапа и копирование на него скриншота
	///HBITMAP bm =::CreateCompatibleBitmap(dstDC, sz.cx, sz.cy);
	oldbm2 = (HBITMAP)::SelectObject(memDC2, bm);
	MoveWindow(0, 0, w, h);
	ShowWindow(SW_SHOW);
	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
}

LRESULT CRegionSelect::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Down = true;
	Start.x = LOWORD(lParam); 
	Start.y = HIWORD(lParam);
	return 0;
}

LRESULT CRegionSelect::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(!Down) return 0;

	HDC dc = GetDC();
	SetROP2(dc, R2_NOTXORPEN);
	SelectObject(dc, pen);
	if(End.x>-1)
		Rectangle(dc, Start.x,Start.y, End.x, End.y);

	End.x = LOWORD(lParam); 
	End.y = HIWORD(lParam);

	Rectangle(dc, Start.x,Start.y, End.x, End.y);

	return 0;
}

LRESULT CRegionSelect::OnEraseBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true;
	return 0;
}

LRESULT CRegionSelect::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Down = false;
	End.x = LOWORD(lParam); 
	End.y = HIWORD(lParam); 
	Finish();
	return 0;
}

LRESULT CRegionSelect::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Hide(false);
	return 0;
}

void CRegionSelect::OnTimer(UINT_PTR nIDEvent)
{
	KillTimer(nIDEvent);

	int Quality, Delay, Format, EntireScr;
	Format = Settings.ScreenshotSettings.Format; 
	Quality = Settings.ScreenshotSettings.Quality; 
	Delay = Settings.ScreenshotSettings.Delay;

	// Выбор в качестве цели всего экрана или активного окна, в зависимости от настроек
	HWND hwnd = GetDesktopWindow();

	RECT r;
	// Расчет размеров изображения в зависмости от размеров и положения окна 
	::GetWindowRect(hwnd, &r);

	int xScreen, yScreen;
	int xshift = 0, yshift = 0;
	xScreen = GetSystemMetrics(SM_CXSCREEN);
	yScreen = GetSystemMetrics(SM_CYSCREEN);
	if(r.right > xScreen)
		r.right = xScreen;
	if(r.bottom > yScreen)
		r.bottom = yScreen;
	if(r.left < 0){
		xshift = -r.left;
		r.left = 0;
	}
	if(r.top < 0){
		yshift = -r.top;
		r.top = 0;
	}

	sz.cx = r.right-r.left;
	sz.cy=r.bottom-r.top;

	// Подготовка контекста рисования
	dstDC = ::GetDC(NULL);
	HDC srcDC = ::GetWindowDC(hwnd);
	memDC = ::CreateCompatibleDC(dstDC);

	// Создание битмапа и копирование на него скриншота
	HBITMAP bm = ::CreateCompatibleBitmap(dstDC, sz.cx, sz.cy);
	oldbm = (HBITMAP)::SelectObject(memDC,bm);
	if(!::BitBlt(memDC, 0, 0, sz.cx, sz.cy, srcDC, xshift, yshift, SRCCOPY|CAPTUREBLT))
		return ;//ScreenshotError();

	TCHAR szBuffer[256];

	::SelectObject(memDC,oldbm);

	ShowW(m_hWnd, bm, sz.cx, sz.cy);
}

bool CRegionSelect::Execute(CRegionSelectCallback *RegionSelectCallback)
{
	m_pCallBack = RegionSelectCallback;
	*m_szFileName = 0;
	if(!m_hWnd)
		Create(Parent);
	ShowWindow(SW_HIDE);

	if(m_hWnd)
		SetTimer(1, 340);

	return false;
}

void CRegionSelect::Finish()
{
	::SelectObject(memDC2, oldbm2 );
	DeleteObject(memDC2);

	int Quality, Delay, Format, EntireScr;
	Format = Settings.ScreenshotSettings.Format; 
	Quality = Settings.ScreenshotSettings.Quality; 
	Delay = Settings.ScreenshotSettings.Delay;


	::SelectObject(memDC, bm);
	int x,y,w,h = 0;

	x = min(Start.x, End.x);
	y = min(Start.y, End.y);

	w = max(Start.x, End.x) - x;
	h = max(Start.y, End.y) - y;

	if(w==0){x=0;y=0;w=sz.cx;h=sz.cy;}

	HDC mem2 = ::CreateCompatibleDC(dstDC);

	HBITMAP bm2 =::CreateCompatibleBitmap(dstDC, w, h);
	HBITMAP oldbm = (HBITMAP)::SelectObject(mem2,bm2);
	if(!::BitBlt(mem2, 0, 0, w, h, memDC, x, y, SRCCOPY|CAPTUREBLT)) 
	{
		Hide(false); 
		return; 
	};

	Bitmap b(bm2,0);
	TCHAR szBuffer[256];

	MySaveImage(&b,_T("screenshot"), m_szFileName, Format, Quality);
	if(m_pCallBack) m_pCallBack->OnScreenshotSaving(m_szFileName,&b);
	// Сохранение изображения в файл, имя возвращается в Filename

	DeleteObject(bm2);
	// Удаление временного битмапа и контекста рисования
	DeleteObject(SelectObject(memDC,oldbm));
	DeleteObject(memDC); 

	Hide();
}

void CRegionSelect::Hide(bool Res)
{
	ShowWindow(SW_HIDE);

	if(m_pCallBack)  
		m_pCallBack->OnScreenshotFinished((int)Res);

	DestroyWindow();
	m_hWnd = 0;
}

void CRegionSelect::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_ESCAPE)  // Cancels screenshoting and shows main window
		Hide(false);

	else if (nChar == VK_RETURN) 
	{
		Start.x = 0;   // If "Enter" key was pressed, we make a shot of entire screen
		Start.y = 0; 
		End.x = 0; 
		End.y = 0;
		Finish();
	}
}

BOOL CRegionSelect::OnSetCursor(CWindow wnd, UINT nHitTest, UINT message)
{
	SetCursor(CrossCursor);
	return TRUE;
}