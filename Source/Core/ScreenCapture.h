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

#ifndef IU_CORE_SCREEN_CAPTURE_H
#define IU_CORE_SCREEN_CAPTURE_H

#include <vector>
#include "atlheaders.h"
#include <gdiplus.h>

bool GetScreenBounds(RECT& rect);
HRGN GetWindowVisibleRegion(HWND wnd);

void TimerWait(int Delay);
enum CaptureMode {cmFullScreen, cmActiveWindow, cmRectangles, cmFreeform, cmWindowHandles };

class CScreenshotRegion
{
	public:
		virtual bool GetImage(HDC src, Gdiplus::Bitmap** res) = 0;
		virtual ~CScreenshotRegion()
		{
		}

		virtual bool PrepareShooting(bool fromScreen)
		{
			m_bFromScreen = fromScreen;
			return true;
		}

		virtual void AfterShooting()
		{
		}

		virtual bool IsEmpty()
		{
			return false;
		}

	protected:
		bool m_bFromScreen;
};

class CRectRegion : public CScreenshotRegion
{
	public:
		CRectRegion();
		CRectRegion(int x, int y, int width, int height);
		CRectRegion(HRGN region);
		virtual bool GetImage(HDC src, Gdiplus::Bitmap** res);
		bool IsEmpty();
		~CRectRegion();
	protected:
		CRgn m_ScreenRegion;
};

class CWindowHandlesRegion : public CRectRegion
{
	public:
		struct WindowCapturingFlags
		{
			bool RemoveCorners;
			bool AddShadow;
			bool RemoveBackground;
		};
		CWindowHandlesRegion();
		CWindowHandlesRegion(HWND wnd);
		void AddWindow(HWND wnd, bool Include);
		void RemoveWindow(HWND wnd);
		void Clear();
		void SetWindowHidingDelay(int delay);
		void setWindowCapturingFlags(WindowCapturingFlags flags);
		virtual bool GetImage(HDC src, Gdiplus::Bitmap** res);
		bool IsEmpty();
		~CWindowHandlesRegion();
	protected:
		struct CWindowHandlesRegionItem
		{
			HWND wnd;
			bool Include;
		};
		Gdiplus::Bitmap* CaptureWithTransparencyUsingDWM();
		HWND topWindow;
		int m_WindowHidingDelay;
		bool m_ClearBackground;
		bool m_RemoveCorners;
		bool m_PreserveShadow;
		std::vector<CWindowHandlesRegionItem> m_hWnds;
};

class CActiveWindowRegion : public CWindowHandlesRegion
{
	public:
		CActiveWindowRegion();
		virtual bool GetImage(HDC src, Gdiplus::Bitmap** res);
};

class CFreeFormRegion : public CRectRegion
{
	public:
		CFreeFormRegion();
		void AddPoint(POINT point);
		void Clear();
		bool IsEmpty();
		virtual bool GetImage(HDC src, Gdiplus::Bitmap** res);
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
		Gdiplus::Bitmap* releaseCapturedBitmap();

	private:
		int m_captureDelay;
		Gdiplus::Bitmap* m_capturedBitmap;
		HBITMAP m_source;
};

#endif  // IU_CORE_SCREEN_CAPTURE_H
