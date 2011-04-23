/*
    Image Uploader - application for uploading images/files to Internet
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

#include "../../atlheaders.h"
#include "RegionSelect.h"
#include "../../Func/common.h"
#include <math.h>
#include "LogWindow.h"
#include "../../Func/LangClass.h"
#include "../../Func/Settings.h"
//#include <Gdipluseffects.h>


struct WindowsListItem
{
	HWND handle;
	RECT rect;
};

std::vector<WindowsListItem> windowsList;


BOOL CALLBACK EnumChildProc(HWND hwnd,	LPARAM lParam)
{
	if(IsWindowVisible(hwnd)){

		EnumChildWindows(hwnd,	EnumChildProc,	0	);
		WindowsListItem newItem;
		newItem.handle = hwnd;
		GetWindowRect(hwnd, &newItem.rect); 
		windowsList.push_back(newItem);
	}
	return true;
}


BOOL CALLBACK RegionEnumWindowsProc(HWND hwnd,LPARAM lParam 	)
{
	if(IsWindowVisible(hwnd))
	{
		EnumChildWindows(hwnd, EnumChildProc, 0);
		WindowsListItem newItem;
		newItem.handle = hwnd;
		GetWindowRect(hwnd, &newItem.rect);
		windowsList.push_back(newItem);
		
	}
	return true;
}

HWND WindowUnderCursor(POINT pt, HWND exclude)
{
	if(windowsList.empty())
	{
		EnumWindows(RegionEnumWindowsProc, 0);
	}
	for(size_t i=0; i<windowsList.size(); i++)
	{
		WindowsListItem &curItem = windowsList[i];
		if(::PtInRect(&curItem.rect, pt) && curItem.handle!=exclude && IsWindowVisible(curItem.handle))

			return curItem.handle;
	}
	return 0;
}



//     Region selection window class
//
//

CRegionSelect RegionSelect;

// CRegionSelect
CRegionSelect::CRegionSelect()
{
	m_bDocumentChanged = false;
	m_bFinish = false;
	m_bSaveAsRegion = false;
	m_ResultRegion = 0;
	RectCount = 0;
	Down = false;
	m_brushSize = 0;
	End.x = -1;
	End.y = -1;
	Start.x = -1;
	Start.y = -1;
	DrawingPen = 0;
	DrawingBrush = 0;
	
	cxOld = -1;
	cyOld = -1;
	pen = CreatePen(PS_SOLID, 2, 0); //Solid black line, width = 2 pixels
	CrossCursor = LoadCursor(NULL,IDC_CROSS);
	HandCursor = LoadCursor(NULL,IDC_HAND);
}


CRegionSelect::~CRegionSelect()
{ 
	delete m_ResultRegion;
	if(DrawingPen) DeleteObject(DrawingPen);
	if(DrawingBrush) DeleteObject(DrawingBrush);
	if(pen)	DeleteObject(pen);
	if(CrossCursor)  DeleteObject(CrossCursor);
	Detach();
}

LRESULT CRegionSelect::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;  // Let the system set the focus
}

LRESULT CRegionSelect::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	BeginPaint(&ps);

	HDC dc = ps.hdc;

	int w = ps.rcPaint.right-ps.rcPaint.left;
	int h = ps.rcPaint.bottom-ps.rcPaint.top;

	if(m_bPictureChanged)
	{
		if(Down) return 0;
		RECT rc;
		GetClientRect(&rc);
		CRgn newRegion;
		newRegion.CreateRectRgn(0,0,rc.right,rc.bottom);
		SelectClipRgn(doubleDC, newRegion);
		BitBlt(doubleDC,ps.rcPaint.left,ps.rcPaint.top,w,h,memDC2,  ps.rcPaint.left,ps.rcPaint.top,SRCCOPY);
		newRegion.CombineRgn(m_SelectionRegion,RGN_DIFF);
		CBrush br;
		SelectClipRgn(doubleDC, newRegion);
		br.CreateSolidBrush(RGB(200,200,0));
		BLENDFUNCTION bf ;
		//настройки прозрачности
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 1;
		bf.SourceConstantAlpha = 40; // прозрачность 50% (0 - 255)
		bf.AlphaFormat = 0;///*0 */ /*AC_SRC_ALPHA*/0;
		
		if(RectCount) 
			if(AlphaBlend(doubleDC, ps.rcPaint.left, ps.rcPaint.top, w,h, alphaDC, ps.rcPaint.left, ps.rcPaint.top, w,h, bf)==FALSE)
			{
				//MessageBox(_T("Alphablend failed"));
			};
		newRegion.DeleteObject();
		newRegion.CreateRectRgn(0,0,rc.right,rc.bottom);
		SelectClipRgn(doubleDC, newRegion);
		RECT SelWndRect;
		if(hSelWnd)
		{
			CRgn WindowRgn = GetWindowVisibleRegion(hSelWnd);
			WindowRgn.OffsetRgn(topLeft);
			WindowRgn.GetRgnBox(&SelWndRect);
			CRect DrawingRect = SelWndRect;
			DrawingRect.DeflateRect(2, 2);
			SelectObject(doubleDC, pen);
			SetROP2(doubleDC, R2_NOTXORPEN);
			SelectClipRgn(doubleDC, 0);
			Rectangle(doubleDC, DrawingRect.left,DrawingRect.top, DrawingRect.right, DrawingRect.bottom);
		}
		m_bPictureChanged = false;
	}
	BitBlt(dc,ps.rcPaint.left,
		ps.rcPaint.top,w,h,doubleDC,ps.rcPaint.left,
		ps.rcPaint.top,SRCCOPY);

	EndPaint(&ps);
	m_bPainted = true;
	return 0;
}

LRESULT CRegionSelect::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Down = true;
	Start.x = LOWORD(lParam); 
	Start.y = HIWORD(lParam);

	POINT newPoint  = {LOWORD(lParam), HIWORD(lParam)};
	m_curvePoints.push_back(newPoint);
	return 0;
}


LRESULT CRegionSelect::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	DWORD fwKeys = wParam;
	POINT point = {cx, cy};
	HWND hNewSelWnd;

	
		/*CRect toolbarRC;
		Toolbar.GetWindowRect(&toolbarRC);
		toolbarRC.InflateRect(30,70);
		bool HideToolBar = true;
		if(toolbarRC.PtInRect(point))
		{
			if(!(fwKeys & MK_LBUTTON) && 	!(fwKeys & MK_RBUTTON))
			{
				HideToolBar = false;
				if(!m_btoolWindowTimerRunning)
				{
					SetTimer(1,400);
					m_btoolWindowTimerRunning = true;
				}
			}
		//do smth	
		}
		else
		{
			if(m_btoolWindowTimerRunning)
			KillTimer(1);
			m_btoolWindowTimerRunning = false;
			Toolbar.ShowWindow(SW_HIDE);
		}*/


	bool bUpdateWindow = false;

	if(m_SelectionMode == smWindowHandles)
	{
		POINT newP=point;
		ClientToScreen(&newP );

      if ((hNewSelWnd = WindowUnderCursor(newP,m_hWnd)) == 0)
        hNewSelWnd = ::GetDesktopWindow();
      else
      {
        /*HWND hChildWnd = MyChildWindowFromPoint(hNewSelWnd, point);
        if ( hChildWnd )
          hNewSelWnd = hChildWnd;*/
      }
		if(hNewSelWnd != hSelWnd)
      {
			CRgn FullScreenRgn;
		

			FullScreenRgn.CreateRectRgnIndirect(&m_screenBounds);
			RECT SelWndRect;
			::GetWindowRect( hNewSelWnd, &SelWndRect );
			CRgn WindowRgn;
			  
			if(::GetWindowRgn(hNewSelWnd, WindowRgn) != ERROR)
			{
				//WindowRegion.GetRgnBox( &WindowRect); 
				WindowRgn.OffsetRgn( SelWndRect.left, SelWndRect.top);
			}
		  
			CBrush br;
			br.CreateSolidBrush(RGB(200,0,0));
		
			m_bPictureChanged = true;
			m_PrevWindowRect = SelWndRect;
		  
			CRgn repaintRgn;
			repaintRgn.CreateRectRgnIndirect(&SelWndRect);
			repaintRgn.OffsetRgn(topLeft);

			if(!m_prevWindowRgn.IsNull())
			repaintRgn.CombineRgn(m_prevWindowRgn, RGN_OR);
			

			
			m_prevWindowRgn = repaintRgn;
			InvalidateRgn(repaintRgn);
			repaintRgn.Detach();
			bUpdateWindow = true;
      }
		hSelWnd = hNewSelWnd;

		if(!(wParam & MK_RBUTTON))
		{
			bUpdateWindow = true;
		}
	}
	//else
	{
		if(fwKeys & MK_LBUTTON && Down	)
		{
			HDC dc = GetDC();
			
			SelectObject(dc, pen);

			if(m_SelectionMode!=smFreeform)
			{
				SetROP2(dc, R2_NOTXORPEN);
				if(End.x>-1)
					Rectangle(dc, Start.x,Start.y, End.x, End.y);

				End.x = LOWORD(lParam); 
				End.y = HIWORD(lParam);

				bool Draw = true;
				if(m_SelectionMode == smWindowHandles)
				{
				
					if(abs(End.x-Start.x)<7 && abs(End.y-Start.y)<7)
					{
						Draw=false;
					}
					else 	
					{
						m_SelectionMode = smRectangles;
						hSelWnd = 0;
						m_bPictureChanged = true;
						/*Invalidate();*/
						bUpdateWindow = true;
					}
				}
					if(Draw) Rectangle(dc, Start.x,Start.y, End.x, End.y);
				
				
			}
			else
			{
				SetROP2(doubleDC, R2_COPYPEN);
				POINT p = m_curvePoints.back();
				MoveToEx(doubleDC, p.x, p.y,0);
				SelectObject(doubleDC, pen);
				POINT newPoint  = {LOWORD(lParam), HIWORD(lParam)};
				LineTo(doubleDC, newPoint.x, newPoint.y);
				m_curvePoints.push_back(newPoint);

				RECT RectToRepaint;
				RectToRepaint.left = min(p.x, newPoint.x) - m_brushSize;
				RectToRepaint.top = min(p.y, newPoint.y) - m_brushSize;
				RectToRepaint.right = max(p.x, newPoint.x) + m_brushSize;
				RectToRepaint.bottom = max(p.y, newPoint.y) + m_brushSize;
				InvalidateRect(&RectToRepaint, false);

			}
			ReleaseDC(dc);
		}
}
		if(wParam & MK_RBUTTON)
		{
			HGDIOBJ oldPen2 = SelectObject(memDC2, DrawingPen);

			if(cxOld != -1)
			{
				SelectClipRgn(memDC2, 0);
				MoveToEx(memDC2, cxOld, cyOld,0);
				LineTo(memDC2, cx,cy);

				RECT RectToRepaint;
				RectToRepaint.left = min(cxOld, cx) - m_brushSize;
				RectToRepaint.top = min(cyOld, cy) - m_brushSize;
				RectToRepaint.right = max(cxOld, cx) + m_brushSize;
				RectToRepaint.bottom = max(cyOld, cy) + m_brushSize;
				CRgn rgn;
				rgn.CreateRectRgnIndirect(&RectToRepaint);
				m_bPictureChanged = true;
				m_bDocumentChanged = true;
				InvalidateRect(&RectToRepaint);
				UpdateWindow();
			}
			cxOld = cx;
			cyOld = cy;	
		}

	if(bUpdateWindow) UpdateWindow();
	return 0;
}

LRESULT CRegionSelect::OnEraseBg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = true; //avoids flickering
	return TRUE;
}

LRESULT CRegionSelect::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(!Down) return 0;
	Down = false;
	End.x = LOWORD(lParam)+1; 
	End.y = HIWORD(lParam)+1;
	CRgn newRegion;	RECT winRect;
	SHORT shiftState = GetAsyncKeyState(VK_SHIFT);

	WORD fwKeys = wParam; 
	m_bPictureChanged = true;

	if(m_SelectionMode == smFreeform) 
	{
		Finish();
		return 0;
	}

	if(m_SelectionMode != smWindowHandles || (abs(End.x-Start.x)>7 && abs(End.y-Start.y)>7))
	newRegion.CreateRectRgn(Start.x,Start.y,End.x,End.y);

	else
	{
		
		::GetWindowRect(hSelWnd, &winRect);

		newRegion.CreateRectRgnIndirect(&winRect);
		newRegion.OffsetRgn(topLeft);
		RECT invRect;
		newRegion.GetRgnBox(&invRect);

		Start.x = invRect.left;
		Start.y = invRect.top;
		End.x = invRect.right;
		End.y = invRect.bottom;
		if(fwKeys & MK_CONTROL)
			m_SelectedWindowsRegion.AddWindow(hSelWnd, false);
		else m_SelectedWindowsRegion.AddWindow(hSelWnd, true);
		//hSelWnd = 0;
	}

	if(fwKeys & MK_CONTROL)	
	{
		m_SelectionRegion.CombineRgn(newRegion, RGN_DIFF);
		
	}
	else if(shiftState& 0x8000) // shift is down
	{
		m_SelectionRegion.CombineRgn(newRegion, RGN_OR);
	}
	else
	{
		m_SelectionRegion.CombineRgn(newRegion, RGN_OR);
		Finish();
		return 0;
	}
	if(!RectCount)
	{
		RectCount++;
		Invalidate();
	}
	else{
	RectCount++;

		RECT RectToRepaint;
		//if(m_SelectionMode != smWindowHandles)
		{
	RectToRepaint.left = min(Start.x, End.x) - m_brushSize;
	RectToRepaint.top = min(Start.y, End.y) - m_brushSize;
	RectToRepaint.right = max(Start.x, End.x) + m_brushSize;
	RectToRepaint.bottom = max(Start.y, End.y) + m_brushSize;
		
InvalidateRect(&RectToRepaint);
		}

	}

	Start.x = -1;
	Start.y = -1;
	End.x = -1;
	End.y = -1;
	return 0;
}

LRESULT CRegionSelect::OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Down = false;
	return 1;
}

bool CRegionSelect::Execute(HBITMAP screenshot, int width, int height)
{
	m_bPictureChanged = false;
	m_bDocumentChanged = false;
	m_btoolWindowTimerRunning = false;
	if(!screenshot) return false;
	m_bFinish = false;
	m_bResult = false;
	Cleanup();

	m_SelectionRegion.CreateRectRgn(0,0,0,0);
	m_curvePoints.clear();

	m_Width = width;
	m_Height = height;
	BITMAPINFOHEADER    bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.biSize        = sizeof(bmi);
    bmi.biWidth        = m_Width;
    bmi.biHeight        = m_Height;
    bmi.biPlanes        = 1;
    bmi.biBitCount        = 4 * 8;
    bmi.biCompression    = BI_RGB;

	HDC dstDC = ::GetDC(0);
	 //doubleBm.
  ///* m_hBitmap = */CreateDIBSection(dstDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, 0, NULL, NULL);

	doubleBm.CreateCompatibleBitmap(dstDC, m_Width, m_Height);
	doubleDC.CreateCompatibleDC(dstDC);
	HBITMAP oldDoubleBm = doubleDC.SelectBitmap(doubleBm);

	m_bmScreenShot = screenshot;
	memDC2.CreateCompatibleDC(dstDC);
	HBITMAP oldBm = memDC2.SelectBitmap(m_bmScreenShot);

	setDrawingParams(Settings.ScreenshotSettings.brushColor, 3);
	Down = false;
	Start.x = -1;
	Start.y = -1;
	End.x = -1;
	End.y = -1;

	cxOld = -1;
	cyOld = -1;
   // HDC hDC;
    //hDC = GetDC(NULL);
	 //alphaBm.
   ///* m_hBitmap =*/ CreateDIBSection(dstDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, 0, NULL, NULL);

	alphaBm.CreateCompatibleBitmap(dstDC, m_Width, m_Height);
	
	CBrush br2;
	br2.CreateSolidBrush(RGB(0,0,150));
	alphaDC.CreateCompatibleDC(dstDC);
	HBITMAP oldAlphaBm = alphaDC.SelectBitmap(alphaBm);
	RECT r = {0, 0, m_Width, m_Height};
	alphaDC.FillRect(&r, br2);	
	m_bPictureChanged = true;
	
	if(!m_hWnd)
	{
		Create(0,r,_T("ImageUploader_RegionWnd"),WS_POPUP,WS_EX_TOPMOST	);
	}

	//RECT screenBounds;
	GetScreenBounds(m_screenBounds);
	MoveWindow(m_screenBounds.left, m_screenBounds.top,m_screenBounds.Width(), m_screenBounds.Height());
	topLeft.x = 0;
	topLeft.y = 0;
	ScreenToClient(&topLeft);

	InvalidateRect(0,FALSE);
	ShowWindow(SW_SHOW);
		
	MSG msg;
	while(!m_bFinish && GetMessage(&msg,NULL,NULL,NULL) ) 
	{  
		TranslateMessage(&msg);    
		DispatchMessage(&msg); 
	}

	memDC2.SelectBitmap(oldBm);
	memDC2.DeleteDC();

	doubleDC.SelectBitmap(oldDoubleBm);
	doubleDC.DeleteDC();

	alphaDC.SelectBitmap(oldAlphaBm);
	alphaDC.DeleteDC();

	alphaBm.DeleteObject();

	::ReleaseDC(0, dstDC);
	ShowWindow(SW_HIDE);
	return m_bResult;
}

void CRegionSelect::Finish()
{
	m_bResult = true;
	if(m_SelectionMode == smRectangles || (m_SelectionMode == smWindowHandles && wasImageEdited()))
	{
		m_ResultRegion = new CRectRegion(m_SelectionRegion);
	}
	else if(m_SelectionMode == smFreeform)
	{
		CFreeFormRegion * newRegion = new CFreeFormRegion();
		for(size_t i=0; i<m_curvePoints.size(); i++)
		{
			newRegion->AddPoint(m_curvePoints[i]);
		}
		m_ResultRegion = newRegion;
	}
	else if(m_SelectionMode == smWindowHandles)
	{
		
			m_ResultRegion = new CWindowHandlesRegion(m_SelectedWindowsRegion);
	}

	
	if(m_ResultRegion->IsEmpty()) 
	{
		m_bResult = false;
		
	}
	

	m_bFinish = true;
}

void CRegionSelect::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_ESCAPE)  // Cancel capturing
	{
		m_bFinish = true;
		m_bResult = false;
	}

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
	if(m_SelectionMode != smWindowHandles)
	SetCursor(CrossCursor);
	else
	SetCursor(HandCursor);
	return TRUE;
}

LRESULT  CRegionSelect::OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	/*RButtonDown=false;
	
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam); 

	if(cxOld == -1)
	{
		HDC dc = m_dc;

		HGDIOBJ oldPen = SelectObject(dc, DrawingPen);
		HGDIOBJ oldPen2 = SelectObject(memDC2, DrawingPen);
		HGDIOBJ oldBrush = SelectObject(dc , DrawingBrush);
		HGDIOBJ oldBrush2 = SelectObject(memDC2, DrawingBrush);
		Ellipse(dc,	cx-1,cy-1,cx+1,cy+1);
		Ellipse(memDC2,	cx-1,cy-1,cx+1,cy+1);
		SelectObject(dc, oldPen);	
		SelectObject(dc, oldBrush);
		SelectObject(memDC2, oldPen);	
		SelectObject(memDC2, oldBrush);
		
	}*/
	cxOld = -1;
	cyOld = -1;
	return 0;
}
bool CRegionSelect::setDrawingParams(COLORREF color, int brushSize)
{
	if(brushSize<1) brushSize = 1;
	
	if(brushSize == m_brushSize && color == m_brushColor) return true;

	if(DrawingPen) DeleteObject(DrawingPen);
	DrawingPen = CreatePen(PS_SOLID, brushSize, color);

	if(DrawingBrush) DeleteObject(DrawingBrush);
	DrawingBrush = CreateSolidBrush(color);

	m_brushSize = brushSize;
	m_brushColor = color;
	return true;
}

LRESULT CRegionSelect::OnMButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND 	hwnd = (HWND) wParam;  
	POINT ClientPoint, ScreenPoint;

	ScreenPoint.x = LOWORD(lParam); 
	ScreenPoint.y = HIWORD(lParam); 
	ClientPoint = ScreenPoint;
	::ScreenToClient(hwnd, &ClientPoint);
	
	CColorDialog ColorDialog(m_brushColor);
	if(ColorDialog.DoModal(m_hWnd) == IDOK)
	{
		COLORREF newColor =  ColorDialog.GetColor();
		setDrawingParams(newColor, m_brushSize);
		return TRUE;
	}
	return 0;
}

LRESULT CRegionSelect::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TCHAR chCharCode = (TCHAR) wParam;
	if(chCharCode == _T('['))
	{
		setDrawingParams(m_brushColor,m_brushSize-1);
	}
	else if(chCharCode == _T(']'))
	{
		setDrawingParams(m_brushColor, m_brushSize+1 );
	}
	return 0;
}

void CRegionSelect::Cleanup()
{
	delete m_ResultRegion;
	m_ResultRegion = 0;
	RectCount = 0 ;
	windowsList.clear();
	//childWindowsList.clear();
	hSelWnd = 0;
	m_bPictureChanged = true;
	m_SelectedWindowsRegion.Clear();
	m_bPainted = false;

	if(!m_SelectionRegion.IsNull())
		m_SelectionRegion.DeleteObject();

	if(!doubleDC.IsNull())
		doubleDC.DeleteDC();

	if(!doubleBm.IsNull())
		doubleBm.DeleteObject();
}

CScreenshotRegion* CRegionSelect::region() const
{
	return m_ResultRegion;
}


LRESULT CRegionSelect::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	int toolbarWidth = 30;
	int toolbarHeight = 200;

	rc.left=0;
	rc.top = (rc.bottom-toolbarHeight)/2;
	rc.bottom -= rc.top;
	rc.right = toolbarWidth;
	/*Toolbar.Create(m_hWnd,rc,_T(""), WS_POPUP|WS_VISIBLE| TBSTYLE_LIST | CCS_NORESIZE|CCS_LEFT |CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
		
	Toolbar.SetButtonStructSize();
	Toolbar.SetButtonSize(40,24);
	
		
		Toolbar.AddButton(45435, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED | TBSTATE_WRAP, -1, TR(""), 0);
		Toolbar.AddButton(45436, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED | TBSTATE_WRAP, 2, TR(""), 0);
		
		Toolbar.AddButton(44365 + 1, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED | TBSTATE_WRAP, 0, _T(""), 0);
		Toolbar.AutoSize();*/
	return 0;
}

LRESULT CRegionSelect::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 1) // timer ID =1
	{
		Toolbar.ShowWindow(SW_SHOW);
		KillTimer(1);
		m_btoolWindowTimerRunning = false;
	}
	return 0;
}

bool CRegionSelect::wasImageEdited()
{
	return m_bDocumentChanged;
}
