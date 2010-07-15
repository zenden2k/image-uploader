#pragma once
#include <gdiplus.h>
// CRegionSelect

bool GetScreenBounds(RECT &rect);
HRGN GetWindowVisibleRegion(HWND wnd);

void TimerWait(int Delay);
enum CaptureMode {cmFullScreen, cmActiveWindow, cmRectangles, cmFreeform, cmWindowHandles };
class CScreenshotRegion
{
public:
	virtual bool GetImage(HDC src, Bitmap ** res)=0;
	virtual ~CScreenshotRegion(){};
	virtual bool PrepareShooting(bool fromScreen)
	{
		m_bFromScreen = fromScreen;
		return true;
	}
	virtual void AfterShooting()
	{
	}
	virtual bool IsEmpty(){return false;}
protected:
	bool m_bFromScreen;
};

class CRectRegion: public CScreenshotRegion
{
public:
	CRectRegion();
	CRectRegion(int x, int y, int width, int height);
	CRectRegion(HRGN region);
	virtual bool GetImage(HDC src, Bitmap ** res);
	bool IsEmpty();
	~CRectRegion();
protected:
	CRgn m_ScreenRegion;
};

struct CWindowHandlesRegionItem
{
	HWND wnd;
	bool Include;
};

class CWindowHandlesRegion: public CRectRegion
{
public:

	CWindowHandlesRegion();
	CWindowHandlesRegion(HWND wnd);
	void AddWindow(HWND wnd, bool Include);
	void RemoveWindow(HWND wnd);
	void Clear();
	void SetWindowHidingDelay(int delay);
	virtual bool GetImage(HDC src, Bitmap ** res);
	bool IsEmpty();
	~CWindowHandlesRegion();
protected:
	HWND topWindow;
	int m_WindowHidingDelay;
	std::vector<CWindowHandlesRegionItem> m_hWnds;
};

class CActiveWindowRegion: public CWindowHandlesRegion
{
public:
	CActiveWindowRegion();
	virtual bool GetImage(HDC src, Bitmap ** res);
};


class CFreeFormRegion: public CRectRegion
{
public:
	CFreeFormRegion();
	void AddPoint(POINT point);
	void Clear();
	bool IsEmpty();
	virtual bool GetImage(HDC src, Bitmap ** res);
	~CFreeFormRegion();
protected:
	std::vector<POINT> m_curvePoints;
};



class CScreenCaptureEngine
{
public:
	CScreenCaptureEngine();
	~CScreenCaptureEngine();
	bool captureScreen();
	void setSource(HBITMAP src);
	bool captureRegion(CScreenshotRegion* region);
	void setDelay(int msec);
	Gdiplus::Bitmap* capturedBitmap();
private:
	int m_captureDelay;
	Gdiplus::Bitmap *m_capturedBitmap;
	HBITMAP m_source;
};
