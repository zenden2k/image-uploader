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

#ifndef IU_CORE_SCREEN_CAPTURE_H
#define IU_CORE_SCREEN_CAPTURE_H

#include <vector>
#include "Core/Utils/CoreTypes.h"
#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
#include "Core/Utils/CoreTypes.h"

HRGN GetWindowVisibleRegion(HWND wnd);

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
        virtual bool GetImage(HDC src, Gdiplus::Bitmap** res) override;
        bool IsEmpty() override;
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
        virtual bool GetImage(HDC src, Gdiplus::Bitmap** res) override;
        virtual bool IsEmpty() override;
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
        virtual bool GetImage(HDC src, Gdiplus::Bitmap** res) override;
};

class CFreeFormRegion : public CRectRegion
{
    public:
        CFreeFormRegion();
        void AddPoint(POINT point);
        void Clear();
        virtual bool IsEmpty() override;
        virtual bool GetImage(HDC src, Gdiplus::Bitmap** res) override;
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
        std::shared_ptr<Gdiplus::Bitmap> capturedBitmap();
        Gdiplus::Bitmap* releaseCapturedBitmap();

    private:
        int m_captureDelay;
        std::shared_ptr<Gdiplus::Bitmap> m_capturedBitmap;
        release_deleter<Gdiplus::Bitmap> capturedBitmapDeleter_;
        /*static */bool capturedBitmapReleased_;
        HBITMAP m_source;
        /*static*/ void capturedBitmapDeleteFunction(Gdiplus::Bitmap* bm);
private:
    DISALLOW_COPY_AND_ASSIGN(CScreenCaptureEngine);
};

#endif  // IU_CORE_SCREEN_CAPTURE_H
