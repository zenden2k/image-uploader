#include "stdafx.h"
#include "ScreenCapture.h"
#include "../common.h"
#include <math.h>
HWND GetTopParent(HWND wnd)
{
	//HWND res;
	if(GetWindowLong(wnd,GWL_STYLE) & WS_CHILD)
	{

		wnd = ::GetParent(wnd);
	}
	return wnd;
}

std::vector<RECT> monitorsRects;
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	if(lprcMonitor)
	{
		monitorsRects.push_back(*lprcMonitor);
	}
	return TRUE;
}

bool GetScreenBounds(RECT &rect)
{
	monitorsRects.clear();
	EnumDisplayMonitors(0,0, MonitorEnumProc, 0);
	CRect result;
	//POINT topLeft = {0,0};
	//POINT bottomRight ={0, 0};
	for(int i=0; i< monitorsRects.size(); i++)
	{
			CRect Bounds = monitorsRects[i];
			
			result.UnionRect(result,Bounds);
			/*if (Bounds.TopLeft().x < topLeft.x) topLeft.x = Bounds.TopLeft().x;
			if (Bounds.TopLeft().y< topLeft.y) topLeft.y = Bounds.TopLeft().y;
			if ((Bounds.TopLeft().x + Bounds.Width()) > bottomRight.x) 
				bottomRight.x = Bounds.TopLeft().x + Bounds.Width();
			if ((Bounds.TopLeft().y + Bounds.Height()) > bottomRight.y) 
				bottomRight.y = Bounds.TopLeft().y + Bounds.Height();
			//if ((screen.Bounds.Y + screen.Bounds.Height) > bottomRight.Y) bottomRight.Y = screen.Bounds.Y + screen.Bounds.Height;
		*/
		}

		rect = result;
		/*rect.left = topLeft.x;
		rect.top = topLeft.y;
		rect.bottom = rect.top + bottomRight.y +abs(topLeft.y);
		rect.right = rect.left + bottomRight.x +abs(topLeft.x);*/

	//return new Rectangle(topLeft.X, topLeft.Y, bottomRight.X + Math.Abs(topLeft.X), bottomRight.Y + Math.Abs(topLeft.Y));

	return true;
}

/*int GetScreenWidth()
{
	HDC dc = GetDC(0);
	int result =  GetDeviceCaps(dc, DESKTOPHORZRES);
	ReleaseDC(0, dc);
	return result;
	/*HWND wnd = GetDesktopWindow();
	RECT rect;
	GetWindowRect(wnd, &rect);*
	//return rect.right;
}

int GetScreenHeight()
{
	HDC dc = GetDC(0);
	int result = GetDeviceCaps(dc, DESKTOPVERTRES	);
	ReleaseDC(0, dc);
	return result;
	/HWND wnd = GetDesktopWindow();
	RECT rect;
	GetWindowRect(wnd, &rect);*
	//return rect.bottom;
}
*/
void average_polyline(std::vector<POINT>& path, std::vector<POINT>& path2, unsigned n);

typedef enum ChannelARGB
{
	Blue = 0,
	Green = 1,
	Red = 2,
	Alpha = 3
};

// hack for stupid GDIplus
void transferOneARGBChannelFromOneBitmapToAnother(Bitmap& source, Bitmap& dest, ChannelARGB sourceChannel, ChannelARGB destChannel )
{
	Rect r( 0, 0, source.GetWidth(),source.GetHeight() );
	BitmapData  bdSrc;
	BitmapData bdDst;
	source.LockBits( &r,  ImageLockModeRead , PixelFormat32bppARGB,&bdSrc);
	dest.LockBits( &r,  ImageLockModeWrite   , PixelFormat32bppARGB , &bdDst);

	BYTE* bpSrc = (BYTE*)bdSrc.Scan0;
	BYTE* bpDst = (BYTE*)bdDst.Scan0;
	bpSrc += (int)sourceChannel;
	bpDst += (int)destChannel;

	for ( int i = r.Height * r.Width; i > 0; i-- )
	{
		*bpDst = *bpSrc;
		if(*bpDst == 0)
		{
			bpDst -=(int)destChannel;
			*bpDst = 0;
			*(bpDst+1) = 0;
			*(bpDst+2) = 0;
			bpDst +=(int)destChannel;
		}
		bpSrc += 4;
		bpDst += 4;
	}
	source.UnlockBits( &bdSrc );
	dest.UnlockBits( &bdDst );
}

void average_polyline(std::vector<POINT>& path, std::vector<POINT>& path2, unsigned n)
{
	if(path.size() > 2)
	{
		std::deque<POINT> tmp;
		unsigned i, j;
		for(j = 0; j < path.size(); j++)
		{
			double x, y;
			tmp.push_back(path[j]);
		}

		for(i = 0; i < n; i++)
		{
			path2.clear();
			for(j = 0; j < tmp.size(); j++)
			{
				POINT p={tmp[j].x, tmp[j].y};
				int d = 1;
				if(j) 
				{
					p.x += tmp[j-1].x;
					p.y += tmp[j-1].y;
					++d;
				}
				if(j + 1 < tmp.size()) 
				{
					p.x += tmp[j+1].x;
					p.y += tmp[j+1].y;
					++d;
				}
				POINT newP = {p.x / d, p.y / d};
				path2.push_back(newP);	
			}

			if(i < n-1)
			{
				tmp.clear();
				for(j = 0; j < path2.size(); j++)
				{
					tmp.push_back(path2[j]);
				}
			}
		}
	}
}

HRGN GetWindowRegion(HWND wnd)
{
	RECT WndRect;
	::GetWindowRect(wnd, &WndRect );
	CRgn WindowRgn;

	WindowRgn.CreateRectRgnIndirect(&WndRect);
	if(::GetWindowRgn(wnd, WindowRgn) != ERROR)
	{
		//WindowRegion.GetRgnBox( &WindowRect); 
		WindowRgn.OffsetRgn( WndRect.left, WndRect.top);
	}
	return WindowRgn.Detach();
}

HRGN GetWindowVisibleRegion(HWND wnd)
{
	CRgn winReg;
	CRect result;

	if(!(GetWindowLong(wnd,GWL_STYLE) & WS_CHILD))
	{
		winReg = GetWindowRegion(wnd);
		return winReg.Detach();
	}
	GetWindowRect(wnd, &result);
	while(GetWindowLong(wnd,GWL_STYLE) & WS_CHILD)
	{
		wnd = GetParent(wnd);
		CRgn parentRgn;
		RECT rc;
		if(GetClientRect(wnd,&rc))
		{
			MapWindowPoints(wnd,0,(POINT*)&rc,2);
			//parentRgn.CreateRectRgnIndirect(&rc);
		}
		result.IntersectRect(&result, &rc);
	}

	winReg.CreateRectRgnIndirect(&result);
	return winReg.Detach(); 
}

CRectRegion::CRectRegion()
{

}

CRectRegion::~CRectRegion()
{
}

CRectRegion::CRectRegion(int x, int y, int width, int height)
{
	if(!m_ScreenRegion.IsNull())
		m_ScreenRegion.DeleteObject();
	m_ScreenRegion.CreateRectRgn(x, y, x+width, y+height);
}

CRectRegion::CRectRegion(HRGN region)
{
	if(!m_ScreenRegion.IsNull())
		m_ScreenRegion.DeleteObject();
	m_ScreenRegion = region;
}

bool CRectRegion::IsEmpty()
{
	CRect rect(0,0,0,0);
	m_ScreenRegion.GetRgnBox(&rect);
	return rect.IsRectEmpty();
}

bool CRectRegion::GetImage(HDC src, Bitmap ** res)
{
	RECT regionBoundingRect;

	//int screenWidth = 	GetScreenWidth();//GetDeviceCaps(src, HORZRES);
	//int screenHeight = GetScreenHeight();//	GetDeviceCaps(src, VERTRES);
	RECT screenBounds;
	GetScreenBounds(screenBounds);
	CRgn FullScreenRgn;
	FullScreenRgn.CreateRectRgnIndirect(&screenBounds);
	if(!m_bFromScreen)
	FullScreenRgn.OffsetRgn(-screenBounds.left,-screenBounds.top);
	else m_ScreenRegion.OffsetRgn(screenBounds.left,screenBounds.top);
	m_ScreenRegion.CombineRgn(FullScreenRgn, RGN_AND);
	m_ScreenRegion.GetRgnBox(&regionBoundingRect);

	int bmWidth = regionBoundingRect.right - regionBoundingRect.left; 
	int bmHeight = regionBoundingRect.bottom  - regionBoundingRect.top; 

	Bitmap *resultBm = new Bitmap(bmWidth, bmHeight);
	Graphics gr(resultBm);

	HDC gdipDC = gr.GetHDC();

	m_ScreenRegion.OffsetRgn( -regionBoundingRect.left, -regionBoundingRect.top);
	SelectClipRgn(gdipDC, m_ScreenRegion);

	if(!::BitBlt(gdipDC, 0, 0, bmWidth, 
		bmHeight, src, regionBoundingRect.left, regionBoundingRect.top, SRCCOPY|CAPTUREBLT))
	{
		gr.ReleaseHDC(gdipDC);
		delete resultBm;
		return false;
	}

	//Each call to the Graphics::GetHDC should be paired with a call to the Graphics::ReleaseHDC 
	gr.ReleaseHDC(gdipDC);
	/*Blur blur;
	BlurParams myBlurParams;
	myBlurParams.expandEdge = TRUE;
	myBlurParams.radius = 5;
	blur.SetParameters(&myBlurParams);
	RECT r={0,0,bmWidth, bmHeight};
	resultBm->ApplyEffect(&blur,&r);*/
	*res = resultBm;
	return true;
}


CWindowHandlesRegion::CWindowHandlesRegion()
{
	topWindow = 0;
	m_WindowHidingDelay = 0;
}

CWindowHandlesRegion::CWindowHandlesRegion(HWND wnd)
{
	CWindowHandlesRegionItem newItem;
	newItem.Include= true;
	newItem.wnd = wnd;
	m_hWnds.push_back(newItem);
}

void CWindowHandlesRegion::SetWindowHidingDelay(int delay)
{
	m_WindowHidingDelay = delay;
}

bool CWindowHandlesRegion::GetImage(HDC src, Bitmap ** res)
{
	if(m_hWnds.empty()) return false;
	RECT captureRect={0,0,0,0};
	if(!m_ScreenRegion.IsNull())
		m_ScreenRegion.DeleteObject();
	m_ScreenRegion.CreateRectRgnIndirect(&captureRect);

	bool move = false;
	if(m_bFromScreen)
	{
		topWindow = GetTopParent(m_hWnds[0].wnd);
		for(int i=1; i<m_hWnds.size(); i++)
		{
			HWND curTopWindow = GetTopParent(m_hWnds[i].wnd);
			if(topWindow != curTopWindow)
			{
				topWindow = 0;
			}
		}
		if(topWindow)
		{
			TCHAR Buffer[MAX_PATH];
			GetClassName(topWindow, Buffer, sizeof(Buffer)/sizeof(TCHAR));
			if(lstrcmpi(Buffer,_T("Shell_TrayWnd")))
			{
				move = true;
				::SetWindowPos(topWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				TimerWait(m_WindowHidingDelay);
			}
			
		}
	}

	for(int i=0; i<m_hWnds.size(); i++)
	{
		CRgn newRegion=GetWindowVisibleRegion(m_hWnds[i].wnd);
		//GetWindowRect(m_hWnds[i].wnd, &captureRect);

		//newRegion.CreateRectRgnIndirect(&captureRect);

		/**if(::GetWindowRgn(m_hWnds[i].wnd, newRegion) != ERROR)
		{
		//WindowRegion.GetRgnBox( &WindowRect); 
		newRegion.OffsetRgn( captureRect.left, captureRect.top);
		}*/
		//else
		//{

		//}
		m_ScreenRegion.CombineRgn(newRegion, m_hWnds[i].Include ? RGN_OR: RGN_DIFF);
		
	}
	CRect scr;
	GetScreenBounds(scr);
	m_ScreenRegion.OffsetRgn(-scr.left,-scr.top);
	bool result = CRectRegion::GetImage(src, res);

	if(topWindow && move)
		::SetWindowPos(topWindow,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	return result;
}

CWindowHandlesRegion::~CWindowHandlesRegion()
{
}

void CWindowHandlesRegion::AddWindow(HWND wnd, bool Include)
{
	CWindowHandlesRegionItem newItem;
	newItem.wnd = wnd;
	newItem.Include = Include;
	RemoveWindow(wnd);
	m_hWnds.push_back(newItem);
}

void CWindowHandlesRegion::RemoveWindow(HWND wnd)
{
	for(int i=0; i< m_hWnds.size(); i++)
	{
		if(m_hWnds[i].wnd == wnd) 
		{
			m_hWnds.erase(m_hWnds.begin()+i); return; 
		}
	}
}

bool CWindowHandlesRegion::IsEmpty()
{
	return m_hWnds.empty();
}
void CWindowHandlesRegion::Clear()
{
	m_hWnds.clear();
}



void TimerWait(int Delay)
{
	HANDLE hTimer = CreateWaitableTimer(0, TRUE, 0);
	LARGE_INTEGER interval;

	interval.QuadPart = -Delay * 10000;
	SetWaitableTimer(hTimer,&interval,0,0,0,0);
	MsgWaitForSingleObject(hTimer, INFINITE);
	CloseHandle(hTimer);
}
CScreenCaptureEngine::CScreenCaptureEngine()
{
	m_capturedBitmap = NULL;
	m_captureDelay = 0;
	m_source = 0;
}

CScreenCaptureEngine::~CScreenCaptureEngine()
{
	delete m_capturedBitmap;
}

bool CScreenCaptureEngine::captureScreen()
{
	CRect screenBounds;
	GetScreenBounds(screenBounds);
	screenBounds.OffsetRect(-screenBounds.left,-screenBounds.top);
	//int screenWidth = GetScreenWidth();//GetSystemMetrics(SM_CXSCREEN);
	//int screenHeight = GetScreenWidth();//GetSystemMetrics(SM_CYSCREEN);
	CRectRegion capturingRegion(screenBounds.left, screenBounds.top, screenBounds.right-screenBounds.left, screenBounds.bottom-screenBounds.top);
	return captureRegion(&capturingRegion);
}

void CScreenCaptureEngine::setDelay(int msec)
{
	m_captureDelay = msec;
}

Gdiplus::Bitmap* CScreenCaptureEngine::capturedBitmap()
{
	return m_capturedBitmap;
}

void CScreenCaptureEngine::setSource(HBITMAP src)
{
	m_source = src;
}
bool CScreenCaptureEngine::captureRegion(CScreenshotRegion* region)
{
	delete m_capturedBitmap;
	m_capturedBitmap = NULL;
	HDC srcDC;
	HDC screenDC = ::GetDC(0); 

	HGDIOBJ oldBm;
	if(m_source)
	{
		HDC sourceDC = CreateCompatibleDC(screenDC);
		srcDC = sourceDC;
		oldBm = SelectObject(srcDC, m_source);
	}
	else
	{
		srcDC = screenDC;
		if(m_captureDelay)
		{
			TimerWait(m_captureDelay);
		}
	}
	region->PrepareShooting(!(bool)m_source);
	bool result =  region->GetImage(srcDC, &m_capturedBitmap);	
	region->AfterShooting();
	if(m_source)
	{
		SelectObject(srcDC, oldBm);
		DeleteDC(srcDC);
	}
	ReleaseDC(0, screenDC);
	return result;
}

CFreeFormRegion::CFreeFormRegion()
{
}

void CFreeFormRegion::AddPoint(POINT point)
{
	m_curvePoints.push_back(point);
}

void CFreeFormRegion::Clear()
{
	m_curvePoints.clear();
}

bool CFreeFormRegion::IsEmpty()
{
	if(m_curvePoints.empty()) return true;
	GraphicsPath grPath;
	std::vector<Point> points;

	POINT prevPoint={-1, -1};
	std::vector<POINT> curveAvgPoints;
	
	average_polyline(m_curvePoints, curveAvgPoints, 29);
	for(int i=0; i<curveAvgPoints.size(); i++)
	{
		points.push_back(Point(curveAvgPoints[i].x,curveAvgPoints[i].y));
	}
	if(points.empty()) return true;
	grPath.AddCurve(&points[0],points.size());
	Rect grPathRect;
	grPath.GetBounds(&grPathRect);

	int bmWidth = grPathRect.GetRight() - grPathRect.GetLeft();
	int bmHeight = grPathRect.GetBottom() - grPathRect.GetTop();

	return !(bmWidth*bmHeight);
}

bool CFreeFormRegion::GetImage(HDC src, Bitmap ** res)
{
	GraphicsPath grPath;
	std::vector<Point> points;

	POINT prevPoint={-1,-1};
	std::vector<POINT> curveAvgPoints;
	average_polyline(m_curvePoints, curveAvgPoints, 29);
	for(int i=0; i<curveAvgPoints.size(); i++)
	{
		points.push_back(Point(curveAvgPoints[i].x,curveAvgPoints[i].y));
	}
	grPath.AddCurve(&points[0],points.size());
	Rect grPathRect;
	grPath.GetBounds(&grPathRect);

	int bmWidth = grPathRect.GetRight() - grPathRect.GetLeft();
	int bmHeight = grPathRect.GetBottom() - grPathRect.GetTop();

	CDC mem2;
	mem2.CreateCompatibleDC(src);
	CBitmap bm2;
	bm2.CreateCompatibleBitmap(src, bmWidth, bmHeight);
	HBITMAP oldBm = mem2.SelectBitmap(bm2);
	!::BitBlt(mem2, 0, 0, bmWidth, bmHeight, src, grPathRect.GetLeft(), grPathRect.GetTop(), SRCCOPY|CAPTUREBLT);
	mem2.SelectBitmap(oldBm);

	Matrix matrix(1.0f, 0.0f, 0.0f, 1.0f, -grPathRect.GetLeft(), -grPathRect.GetTop());
	grPath.Transform(&matrix);

	Bitmap b(bm2,0);

	SolidBrush   gdipBrush(Color(255,0,0,0));
	Bitmap alphaBm(bmWidth, bmHeight, PixelFormat32bppARGB);

	Graphics alphaGr(&alphaBm);
	alphaGr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
	alphaGr.SetSmoothingMode(SmoothingModeAntiAlias);
	alphaGr.FillPath(&gdipBrush, &grPath);

	Bitmap *finalbm = new Bitmap(bmWidth, bmHeight, PixelFormat32bppARGB);
	Graphics gr(finalbm);
	gr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
	gr.SetSmoothingMode(SmoothingModeAntiAlias);

	gr.DrawImage(&b, 0,0);
	SolidBrush   gdipBrush2(Color(100,123,0,0));
	Pen pn(Color(255,40,255), 1.0f) ;
	Pen pn2(Color(40,0,255), 1.0f) ;
	transferOneARGBChannelFromOneBitmapToAnother(alphaBm, *finalbm,Alpha,Alpha);
	*res = finalbm;
	return true;
}

CFreeFormRegion::~CFreeFormRegion()
{
}

CActiveWindowRegion::CActiveWindowRegion()
{

}

bool CActiveWindowRegion::GetImage(HDC src, Bitmap ** res)
{
	Clear();
	AddWindow(GetForegroundWindow(), true);
	return CWindowHandlesRegion::GetImage(src, res);
}