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

#include "Core/ScreenCapture.h"

#include <cmath>
#include <deque>
#include <Dwmapi.h>
#include "atlheaders.h"
#include "Func/common.h"
#include "Func/MyUtils.h"
#include "Func/WinUtils.h"
#include "resource.h"
#include "Core/Images/Utils.h"
#include "Core/Logging.h"
#include <assert.h>

typedef HRESULT (WINAPI * DwmGetWindowAttribute_Func)(HWND, DWORD, PVOID, DWORD);
typedef HRESULT (WINAPI * DwmIsCompositionEnabled_Func)(BOOL*);
RECT MaximizedWindowFix(HWND handle, RECT windowRect);

using namespace Gdiplus;

void ProcessEvents(void)
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool IsWindowMaximized(HWND handle)
{
    WINDOWPLACEMENT wp;
    GetWindowPlacement(handle, &wp);
    return wp.showCmd == (int)SW_MAXIMIZE;
}

void ActivateWindowRepeat(HWND handle, int count)
{
    for (int i = 0; GetForegroundWindow() != handle && i < count; i++)
    {
        BringWindowToTop(handle);
        Sleep(1);
        ProcessEvents();
    }
}

BOOL MyGetWindowRect(HWND hWnd, RECT* res, bool MaximizedFix = true)
{
    if (!WinUtils::IsVista())
    {
        return GetWindowRect(hWnd, res);
    }
    static HMODULE DllModule =  LoadLibrary(_T("dwmapi.dll"));
    if (DllModule)
    {
        DwmIsCompositionEnabled_Func IsCompEnabledFunc = reinterpret_cast<DwmIsCompositionEnabled_Func>( GetProcAddress(
              DllModule, "DwmIsCompositionEnabled"));
        if (IsCompEnabledFunc)
        {
            BOOL isEnabled = false;
            if (S_OK == IsCompEnabledFunc( &isEnabled))
                if (isEnabled)
                {
                    DwmGetWindowAttribute_Func Func = reinterpret_cast<DwmGetWindowAttribute_Func>(GetProcAddress(DllModule, "DwmGetWindowAttribute"));
                    if (Func)
                    {
                        if (S_OK == Func( hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, res, sizeof(RECT)))
                        {
                            if (MaximizedFix)
                                *res = MaximizedWindowFix(hWnd, *res);
                            return TRUE;
                        }
                    }
                }
        }
    }
    return GetWindowRect(hWnd, res);
}

HRGN CloneRegion(HRGN source)
{
    HRGN resultRgn = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(resultRgn, source, resultRgn, RGN_OR);
    return resultRgn;
}

HWND GetTopParent(HWND wnd)
{
    // HWND res;
// while(GetParent(wnd)!=(HWND)0)
    if (GetWindowLong(wnd, GWL_STYLE) & WS_CHILD)
    {
        wnd = ::GetParent(wnd);
    }
    return wnd;
}

std::vector<RECT> monitorsRects;
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    if (lprcMonitor)
    {
        monitorsRects.push_back(*lprcMonitor);
    }
    return TRUE;
}
bool GetScreenBounds(RECT& rect)
{
    monitorsRects.clear();
    EnumDisplayMonitors(0, 0, MonitorEnumProc, 0);
    CRect result;
    for (size_t i = 0; i < monitorsRects.size(); i++)
    {
        CRect Bounds = monitorsRects[i];
        result.UnionRect(result, Bounds);
    }
    rect = result;
    return true;

}
RECT ScreenFromRectangle(RECT rc)
{
    monitorsRects.clear();
    EnumDisplayMonitors(0, 0, MonitorEnumProc, 0);
    CRect result;
    int max = 0;
    size_t iMax = 0;
    for (size_t i = 0; i < monitorsRects.size(); i++)
    {
        CRect Bounds = monitorsRects[i];
        Bounds.IntersectRect(&Bounds, &rc);
        if (Bounds.Width() * Bounds.Height() > max)
        {
            max = Bounds.Width() * Bounds.Height();
            iMax = i;
        }
        // result.UnionRect(result,Bounds);
    }
    return monitorsRects[iMax];
}

RECT MaximizedWindowFix(HWND handle, RECT windowRect)
{
    RECT res = windowRect;
    if (IsWindowMaximized(handle))
    {
        RECT screenRect = ScreenFromRectangle(windowRect);
        if (windowRect.left < screenRect.left)
        {
            windowRect.right -= (screenRect.left - windowRect.left) /** 2*/;
            windowRect.left = screenRect.left;
        }
        if (windowRect.top < screenRect.top)
        {
            windowRect.bottom -= (screenRect.top - windowRect.top) /** 2*/;
            windowRect.top = screenRect.top;
        }
        IntersectRect(&res, &windowRect, &screenRect);
        //  windowRect.Intersect(screenRect);
    }
    return res;
}


void average_polyline(std::vector<POINT>& path, std::vector<POINT>& path2, unsigned n);

enum ChannelARGB {
    Blue = 0,
    Green = 1,
    Red = 2,
    Alpha = 3
};

// hack for stupid GDIplus
void transferOneARGBChannelFromOneBitmapToAnother(Bitmap& source, Bitmap& dest, ChannelARGB sourceChannel,
                                                  ChannelARGB destChannel )
{
    Rect r( 0, 0, source.GetWidth(), source.GetHeight() );
    BitmapData bdSrc;
    BitmapData bdDst;
    source.LockBits( &r,  ImageLockModeRead, PixelFormat32bppARGB, &bdSrc);
    dest.LockBits( &r,  ImageLockModeWrite, PixelFormat32bppARGB, &bdDst);
    BYTE* bpSrc = reinterpret_cast<BYTE*>(bdSrc.Scan0);
    BYTE* bpDst = reinterpret_cast<BYTE*>(bdDst.Scan0);
    bpSrc += (int)sourceChannel;
    bpDst += (int)destChannel;
    for ( int i = r.Height * r.Width; i > 0; i-- )
    {
        *bpDst = *bpSrc;
        if (*bpDst == 0)
        {
            bpDst -= (int)destChannel;
            *bpDst = 0;
            *(bpDst + 1) = 0;
            *(bpDst + 2) = 0;
            bpDst += (int)destChannel;
        }
        bpSrc += 4;
        bpDst += 4;
    }
    source.UnlockBits( &bdSrc );
    dest.UnlockBits( &bdDst );
}

void average_polyline(std::vector<POINT>& path, std::vector<POINT>& path2, unsigned n)
{
    if (path.size() > 2)
    {
        std::deque<POINT> tmp;
        unsigned i, j;
        for (j = 0; j < path.size(); j++)
        {
            tmp.push_back(path[j]);
        }
        for (i = 0; i < n; i++)
        {
            path2.clear();
            for (j = 0; j < tmp.size(); j++)
            {
                POINT p = {tmp[j].x, tmp[j].y};
                int d = 1;
                if (j)
                {
                    p.x += tmp[j - 1].x;
                    p.y += tmp[j - 1].y;
                    ++d;
                }
                if (j + 1 < tmp.size())
                {
                    p.x += tmp[j + 1].x;
                    p.y += tmp[j + 1].y;
                    ++d;
                }
                POINT newP = {p.x / d, p.y / d};
                path2.push_back(newP);
            }
            if (i < n - 1)
            {
                tmp.clear();
                for (j = 0; j < path2.size(); j++)
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
    ::MyGetWindowRect(wnd, &WndRect );
    CRgn WindowRgn;
    WindowRgn.CreateRectRgnIndirect(&WndRect);
    if (::GetWindowRgn(wnd, WindowRgn) != ERROR)
    {
        // WindowRegion.GetRgnBox( &WindowRect);
        WindowRgn.OffsetRgn( WndRect.left, WndRect.top);
    }
    return WindowRgn.Detach();
}

HRGN GetWindowVisibleRegion(HWND wnd)
{
    CRgn winReg;
    CRect result;
    if (!(GetWindowLong(wnd, GWL_STYLE) & WS_CHILD))
    {
        winReg = GetWindowRegion(wnd);
        return winReg.Detach();
    }
    MyGetWindowRect(wnd, &result);
    while (GetWindowLong(wnd, GWL_STYLE) & WS_CHILD)
    {
        wnd = GetParent(wnd);
        CRgn parentRgn;
        RECT rc;
        if (GetClientRect(wnd, &rc))
        {
            MapWindowPoints(wnd, 0, reinterpret_cast<POINT*>(&rc), 2);
            // parentRgn.CreateRectRgnIndirect(&rc);
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
    if (!m_ScreenRegion.IsNull())
        m_ScreenRegion.DeleteObject();
    m_ScreenRegion.CreateRectRgn(x, y, x + width, y + height);
}

CRectRegion::CRectRegion(HRGN region)
{
    if (!m_ScreenRegion.IsNull())
        m_ScreenRegion.DeleteObject();
    m_ScreenRegion = region;
}

bool CRectRegion::IsEmpty()
{
    CRect rect(0, 0, 0, 0);
    m_ScreenRegion.GetRgnBox(&rect);
    return rect.IsRectEmpty() != 0;
}

bool CRectRegion::GetImage(HDC src, Bitmap** res)
{
    RECT regionBoundingRect;
    CRgn screenRegion = CloneRegion(m_ScreenRegion);
    RECT screenBounds;

    GetScreenBounds( screenBounds );
    CRgn FullScreenRgn;
    FullScreenRgn.CreateRectRgnIndirect(&screenBounds);
    if ( !m_bFromScreen ) {
        FullScreenRgn.OffsetRgn(-screenBounds.left, -screenBounds.top);
    }
    else screenRegion.OffsetRgn(screenBounds.left, screenBounds.top);
    screenRegion.CombineRgn(FullScreenRgn, RGN_AND);
    screenRegion.GetRgnBox(&regionBoundingRect);
    int bmWidth = regionBoundingRect.right - regionBoundingRect.left;
    int bmHeight = regionBoundingRect.bottom  - regionBoundingRect.top;
    
    CBitmap tempBm; // Temporary bitmap and device context
    CDC tempDC;    //  which were added to avoid artefacts with BitBlt
    HDC dc = GetDC( 0 );
    tempBm.CreateCompatibleBitmap( dc, bmWidth, bmHeight );
    tempDC.CreateCompatibleDC( dc );
    HBITMAP oldBm = tempDC.SelectBitmap( tempBm );
    ReleaseDC( 0, dc );
    
    screenRegion.OffsetRgn( -regionBoundingRect.left, -regionBoundingRect.top);

    if (!::BitBlt(tempDC, 0, 0, bmWidth,
                  bmHeight, src, regionBoundingRect.left, regionBoundingRect.top, SRCCOPY | CAPTUREBLT)) {
        return false;
    }

    Bitmap* resultBm = new Bitmap(bmWidth, bmHeight, PixelFormat32bppARGB);
    Graphics gr( resultBm );

    Bitmap srcBm( tempBm, 0);
    gr.SetClip( screenRegion );
    
    // Each call to the Graphics::GetHDC should be paired with a call to the Graphics::ReleaseHDC
    gr.DrawImage( &srcBm, 0, 0);
    gr.Flush();
    tempDC.SelectBitmap( oldBm );
    *res = resultBm;
    return true;
}

CWindowHandlesRegion::CWindowHandlesRegion()
{
    topWindow = 0;
    m_WindowHidingDelay = 0;
    m_ClearBackground = false;
    m_RemoveCorners = true;
    m_PreserveShadow = true;
}

CWindowHandlesRegion::CWindowHandlesRegion(HWND wnd)
{
    CWindowHandlesRegionItem newItem;
    newItem.Include = true;
    newItem.wnd = wnd;
    m_hWnds.push_back(newItem);
}

void CWindowHandlesRegion::SetWindowHidingDelay(int delay)
{
    m_WindowHidingDelay = delay;
}

void CWindowHandlesRegion::setWindowCapturingFlags(WindowCapturingFlags flags)
{
    m_ClearBackground = flags.RemoveBackground;
    m_RemoveCorners = flags.RemoveCorners;
    m_PreserveShadow = flags.AddShadow;
}

COLORREF bgColor;

bool AreImagesEqual(Bitmap* b1, Bitmap* b2)
{
    bool result = true;
    int width = b1->GetWidth();
    int height = b1->GetHeight();
    Rect rect(0, 0, width, height);
    BitmapData b1Data;
    b1->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &b1Data);
    BitmapData b2Data;
    b2->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &b2Data);
    assert(sizeof(unsigned long*) == 4);
    unsigned long* pImage1 = reinterpret_cast<unsigned long*>(b1Data.Scan0);
    unsigned long* pImage2 = reinterpret_cast<unsigned long*>(b2Data.Scan0);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (*(pImage1++) != *(pImage2++))
            {
                result = false;
                break;
            }
        }
    }
    b1->UnlockBits(&b1Data);
    b2->UnlockBits(&b2Data);
    return result;
}

bool ComputeOriginal(Bitmap* whiteBGImage, Bitmap* blackBGImage, Bitmap** out)
{
    assert(whiteBGImage);
    assert(blackBGImage);
    assert(out);

    int width = whiteBGImage->GetWidth();
    int height = whiteBGImage->GetHeight();
    Bitmap* resultImage = new Bitmap(width, height, PixelFormat32bppARGB);
    Gdiplus::Rect rect(0, 0, blackBGImage->GetWidth(), blackBGImage->GetHeight());
    // Access the image data directly for faster image processing
    BitmapData blackImageData;
    blackBGImage->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &blackImageData);
    BitmapData whiteImageData;
    whiteBGImage->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, &whiteImageData);
    BitmapData resultImageData;
    resultImage->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &resultImageData);
    void* pBlackImage = blackImageData.Scan0;
    void* pWhiteImage = whiteImageData.Scan0;
    void* pResultImage = resultImageData.Scan0;
    unsigned char* blackBGImageRGB = ( unsigned char*)pBlackImage /*new unsigned char[bytes]*/;
    unsigned char* whiteBGImageRGB = ( unsigned char*)pWhiteImage /*new unsigned char[bytes]*/;
    unsigned char* resultImageRGB = ( unsigned char*)pResultImage /* new unsigned char[bytes]*/;
    int offset = 0;
    int b0, g0, r0, b1, g1, r1, alphaR, alphaG, alphaB, resultR, resultG, resultB;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // ARGB is in fact BGRA (little endian)
            b0 = blackBGImageRGB[offset + 0];
            g0 = blackBGImageRGB[offset + 1];
            r0 = blackBGImageRGB[offset + 2];
            b1 = whiteBGImageRGB[offset + 0];
            g1 = whiteBGImageRGB[offset + 1];
            r1 = whiteBGImageRGB[offset + 2];
            alphaR = r0 - r1 + 255;
            alphaG = g0 - g1 + 255;
            alphaB = b0 - b1 + 255;
            if (alphaG != 0)
            {
                resultR = r0 * 255 / alphaG;
                resultG = g0 * 255 / alphaG;
                resultB = b0 * 255 / alphaG;
            }
            else
            {
                // Could be any color since it is fully transparent.
                resultR = 255;
                resultG = 255;
                resultB = 255;
            }
            resultImageRGB[offset + 3] = static_cast<byte>(alphaR);
            resultImageRGB[offset + 2] = static_cast<byte>(resultR);
            resultImageRGB[offset + 1] = static_cast<byte>(resultG);
            resultImageRGB[offset + 0] = static_cast<byte>(resultB);
            offset += 4;
        }
    }
    blackBGImage->UnlockBits(&blackImageData);
    whiteBGImage->UnlockBits(&whiteImageData);
    // whiteBGImage2->UnlockBits(&whiteImageData2);
    resultImage->UnlockBits(&resultImageData);
    *out = resultImage;
    return true;
}

void OnEraseBgrnd(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hWnd, &ps);
    HBRUSH br = CreateSolidBrush(bgColor);
    RECT rc;
    GetClientRect(hWnd, &rc);
    FillRect(dc, &rc, br);
    EndPaint(hWnd, &ps);
    DeleteObject(br);
}

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    // If the user wants to close the application
    case WM_DESTROY:
        // then close it
        // PostQuitMessage(WM_QUIT);
        break;

    case WM_PAINT:
        OnEraseBgrnd(hWnd);
        return 1;
        break;

    default:
        // Process the left-over messages
        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }
    // If something was not done, let it go
    return 0;
}

HWND CreateDummyWindow(RECT rc)
{
    HWND hWnd;
    WNDCLASSEX WndClsEx;
    TCHAR* clsName = _T("DummyWindow");
    // Create the application window
    WndClsEx.cbSize        = sizeof(WNDCLASSEX);
    WndClsEx.style         = CS_HREDRAW | CS_VREDRAW;
    WndClsEx.lpfnWndProc   = WndProcedure;
    WndClsEx.cbClsExtra    = 0;
    WndClsEx.cbWndExtra    = 0;
    WndClsEx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    WndClsEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WndClsEx.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    WndClsEx.lpszMenuName  = NULL;
    WndClsEx.lpszClassName = clsName;
    WndClsEx.hInstance     = GetModuleHandle(0);
    WndClsEx.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
    // Register the application
    RegisterClassEx(&WndClsEx);
    // Create the window object
    hWnd = CreateWindowEx( WS_EX_TOPMOST, clsName,
                           _T(""), WS_POPUP, rc.left, rc.top,
                           rc.right - rc.left,
                           rc.bottom - rc.top,
                           NULL,
                           NULL,
                           GetModuleHandle(0),
                           NULL);
    if ( !hWnd )
        return 0;
    return hWnd;
}

BOOL BringWindowToForeground(HWND hWnd)
{
    DWORD dwTimeout;
    ::SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwTimeout, 0);
    ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, 0, 0);
    BOOL bNeedTopmost = !(::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
    if (bNeedTopmost)
        ::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    HWND hCurWnd;
    for (int i = 0; (i < 10) && ((hCurWnd = ::GetForegroundWindow()) != hWnd); i++)
    {
        int nMyTID  = ::GetCurrentThreadId();
        int nCurTID = ::GetWindowThreadProcessId(hCurWnd, 0);
        ::AttachThreadInput(nMyTID, nCurTID, TRUE);
        ::SetForegroundWindow(hWnd);
        ::AttachThreadInput(nMyTID, nCurTID, FALSE);
        Sleep(20);
    }
    if (bNeedTopmost)
        ::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ::SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &dwTimeout, 0);
    return TRUE;
}

enum Corner { TopLeft, TopRight, BottomLeft, BottomRight };

// Removes a pixel from the clipping region of the given graphics object, if
// the bitmap is red at the coordinates of the pixel, or if it is null.
// </summary>
// <param name="bmp">The bitmap with the form corners masked in red</param>

void RemoveCornerPixel(Bitmap* bmp, Graphics* g, int y, int x)
{
    bool remove;
    if (bmp != 0)
    {
        Color color;
        bmp->GetPixel(x, y, &color);
        // detect a shade of red (the color is darker because of the window's shadow)
        remove = (color.GetR() > 0 && color.GetG() == 0 && color.GetB() == 0);
    }
    else
    {
        remove = true;
    }
    if (remove)
    {
        Region region (Rect(x, y, 1, 1));
        g->SetClip(&region, CombineModeExclude);
    }
}

// / <summary>
// / Removes a corner from the clipping region of the given graphics object.
// / </summary>
// / <param name="bmp">The bitmap with the form corners masked in red</param>
void RemoveCorner(Bitmap* bmp, Graphics* g, int minx, int miny, int maxx, Corner corner)
{
    int s1[5] = { 5, 3, 2, 1, 1 };
    int s2[5] = { 1, 1, 2, 3, 5 };
    int* shape;
    if (corner == TopLeft || corner == TopRight)
    {
        shape = s1;
    }
    else
    {
        shape = s2;
    }
    int maxy = miny + 5;
    if (corner == TopLeft || corner == BottomLeft)
    {
        for (int y = miny; y < maxy; y++)
        {
            for (int x = minx; x < minx + shape[y - miny]; x++)
            {
                RemoveCornerPixel(bmp, g, y, x);
            }
        }
    }
    else
    {
        for (int y = miny; y < maxy; y++)
        {
            for (int x = maxx - 1; x >= maxx - shape[y - miny]; x--)
            {
                RemoveCornerPixel(bmp, g, y, x);
            }
        }
    }
}

bool RemoveCorners(Bitmap* windowImage, Bitmap* redBGImage, Bitmap** outResult)
{
    const int cornerSize = 5;
    if (windowImage->GetWidth() > cornerSize * 2 && windowImage->GetHeight() > cornerSize * 2)
    {
        Bitmap* result = new Bitmap(windowImage->GetWidth(),  windowImage->GetHeight(), PixelFormat32bppARGB);
        Graphics g(result);
        g.Clear(Color::Transparent);
        // Remove the transparent pixels in the four corners
        RemoveCorner(redBGImage, &g, 0, 0, cornerSize, TopLeft);
        RemoveCorner(redBGImage, &g, windowImage->GetWidth() - cornerSize, 0, windowImage->GetWidth(), TopRight);
        RemoveCorner(redBGImage, &g, 0, windowImage->GetHeight() - cornerSize, cornerSize, BottomLeft);
        RemoveCorner(redBGImage, &g, windowImage->GetWidth() - cornerSize,
                     windowImage->GetHeight() - cornerSize, windowImage->GetWidth(), BottomRight);
        g.DrawImage(windowImage, 0, 0);
        *outResult = result;
        return true;
    }
    return false;
}

void DrawShadow(Graphics& g, Bitmap* shadowBitmap, int x, int y, int width, int height)
{
    TextureBrush brush(shadowBitmap);
    Bitmap bmpTemp(width, height, PixelFormat32bppARGB);
    Graphics gTemp(&bmpTemp);
    {
        // Draw on a temp bitmap with (0,0) offset, because the texture starts at (0,0)
        gTemp.FillRectangle(&brush, 0, 0, width, height);
        g.DrawImage(&bmpTemp, x, y);
    }
}

bool AddBorderShadow(Bitmap* input, bool roundedShadowCorners, Bitmap** out)
{
    int width = input->GetWidth();
    int height = input->GetHeight();
    Color c;
    input->GetPixel(0, 0, &c);
    bool topLeftRound = c.GetAlpha() < 20;
    input->GetPixel(width - 1, 0, &c);
    bool topRightRound = c.GetAlpha() < 20;
    input->GetPixel(0, height - 1, &c);
    //bool bottomLeftRound = c.GetAlpha() < 20;
    input->GetPixel(width - 1, height - 1, &c);
    //bool bottomRightRound = c.GetAlpha() < 20;
    Bitmap* leftShadow = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_leftShadow), _T("PNG")); // Resources.leftShadow;
    Bitmap* rightShadow = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_rightShadow), _T("PNG"));
    Bitmap* topShadow = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_topShadow), _T("PNG"));
    Bitmap* bottomShadow = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_bottomShadow), _T("PNG"));
    Bitmap* topLeftShadow =
       BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(
                             topLeftRound ? IDR_topLeftShadow : IDR_topLeftShadowSquare), _T("PNG"));
    Bitmap* topRightShadow =
       BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(
                             topRightRound ? IDR_topRightShadow : IDR_topRightShadowSquare), _T("PNG"));
    Bitmap* bottomLeftShadow = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_bottomLeftShadow), _T("PNG"));
    Bitmap* bottomRightShadow = BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDR_bottomRightShadow), _T("PNG"));
    int leftMargin = leftShadow->GetWidth();
    int rightMargin = rightShadow->GetWidth();
    int topMargin = topShadow->GetHeight();
    int bottomMargin = bottomShadow->GetHeight();
    int resultWidth = leftMargin + width + rightMargin;
    int resultHeight = topMargin + height + bottomMargin;
    if (resultHeight - topRightShadow->GetHeight() - bottomRightShadow->GetHeight() <= 0
        || resultWidth - bottomLeftShadow->GetWidth() - bottomRightShadow->GetWidth() <= 0)
    {
        *out = input->Clone(0, 0, input->GetWidth(), input->GetHeight(), PixelFormat32bppARGB);
    }
    else
    {
        Bitmap* bmpResult = new Bitmap(resultWidth, resultHeight, PixelFormat32bppARGB);
        Graphics g(bmpResult);
        g.DrawImage(topLeftShadow, 0, 0);
        g.DrawImage(topRightShadow, resultWidth - topRightShadow->GetWidth(), 0);
        g.DrawImage(bottomLeftShadow, 0, resultHeight - bottomLeftShadow->GetHeight());
        g.DrawImage(bottomRightShadow,
                    (float)resultWidth - bottomRightShadow->GetWidth(), (float)resultHeight -
                    bottomRightShadow->GetHeight());
        DrawShadow(g, leftShadow, 0, topLeftShadow->GetHeight(),
                   leftShadow->GetWidth(), resultHeight - topLeftShadow->GetHeight() - bottomLeftShadow->GetHeight());
        DrawShadow(g, rightShadow, resultWidth - rightShadow->GetWidth(), topRightShadow->GetHeight(),
                   rightShadow->GetWidth(), resultHeight - topRightShadow->GetHeight() - bottomRightShadow->GetHeight());
        DrawShadow( g, topShadow, topLeftShadow->GetWidth(), 0,
                    resultWidth - topLeftShadow->GetWidth() - topRightShadow->GetWidth(), topShadow->GetHeight());
        DrawShadow(g, bottomShadow, bottomLeftShadow->GetWidth(), resultHeight - bottomShadow->GetHeight(),
                   resultWidth - bottomLeftShadow->GetWidth() - bottomRightShadow->GetWidth(), bottomShadow->GetHeight());
        g.DrawImage(input, leftMargin, topMargin);
        *out = bmpResult;
    }
    delete leftShadow ;
    delete rightShadow ;
    delete topShadow;
    delete bottomShadow;
    delete topLeftShadow ;
    delete topRightShadow ;
    delete bottomLeftShadow;
    delete bottomRightShadow;
    return *out != 0;
}

bool CheckRect(RECT rect, COLORREF color)
{
    HDC screenDC = ::GetDC(0);
    COLORREF pixel = GetPixel(screenDC, (rect.right - rect.left) / 2, (rect.bottom - rect.bottom / 2) - 1);
    bool result = pixel == color;
    ReleaseDC(0, screenDC);
    return result;
}

Bitmap* CWindowHandlesRegion::CaptureWithTransparencyUsingDWM()
{
    Bitmap* resultBm = 0;
    Bitmap* original = 0;
    Bitmap* redBgBitmap = 0;
    bool move = false;
    HWND target = topWindow;
    TCHAR Buffer[MAX_PATH];
    GetClassName(target, Buffer, sizeof(Buffer) / sizeof(TCHAR));
    if (lstrcmpi(Buffer, _T("Shell_TrayWnd")))
    {
        BringWindowToForeground(target);
        move = true;
    }
    SetForegroundWindow(target);
    // CRgn newRegion=GetWindowVisibleRegion(target);
    CRect actualWindowRect;
    MyGetWindowRect(target, &actualWindowRect, false);
//    bool IsSimpleRectWindow = newRegion.GetRgnBox(&windowRect)==SIMPLEREGION;
    bgColor = RGB(255, 255, 255);
    HWND wnd = 0;
    CScreenCaptureEngine eng;
    CRectRegion reg(m_ScreenRegion);
    Bitmap* bm1 = 0;
    Bitmap* bm2 = 0;
    Bitmap* bm3 = 0;
    HTHUMBNAIL thumb = 0;
    if (m_ClearBackground || m_RemoveCorners || m_PreserveShadow)
    {
        wnd = CreateDummyWindow(actualWindowRect);
    }
    if (wnd)
    {
        int i = 0;
        while (!CheckRect(actualWindowRect, bgColor))
        {
            WinUtils::TimerWait(50);
            if (i++ > 10) break;
        }
        if (DwmRegisterThumbnail(wnd, m_hWnds[0].wnd, &thumb) != S_OK)
        {
            DestroyWindow(wnd);
            return 0;
        }
        if (m_ClearBackground)
            ShowWindow(wnd, SW_SHOWNOACTIVATE);
        SIZE size;
        if (DwmQueryThumbnailSourceSize(thumb, &size) != S_OK)
            return 0;
        DWM_THUMBNAIL_PROPERTIES props;
        props.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION | DWM_TNP_OPACITY;
        props.fVisible = true;
        props.opacity = 255;
        RECT rcDest = {0, 0, size.cx, size.cy};
        props.rcDestination = rcDest;
        DwmUpdateThumbnailProperties(thumb, &props);
        ProcessEvents();
        eng.captureRegion(&reg);
        bm1 = eng.releaseCapturedBitmap();
    }
    if (m_ClearBackground)
    {
        bgColor = RGB(0, 0, 0);
        ::InvalidateRect(wnd, NULL, true);
        ProcessEvents();
        DwmFlush();
        eng.captureRegion(&reg);
        bm2 = eng.releaseCapturedBitmap();
    }
    if (m_ClearBackground || m_RemoveCorners)
    {
        ShowWindow(wnd, SW_SHOWNOACTIVATE);
        bgColor = RGB(255, 255, 255);
        ::InvalidateRect(wnd, NULL, true);
        ProcessEvents();
        eng.captureRegion(&reg);
        bm3 = eng.releaseCapturedBitmap();
    }
    Bitmap* preResult = 0;
    if (m_ClearBackground)
    {
        if (AreImagesEqual(bm1, bm3))
        {
            ComputeOriginal(bm1, bm2, &original);
            if (original)
            {
                preResult = original;
            }
            else
            {
                assert(bm3);
                preResult = bm3;
                bm3 = 0;
            }
        }
        else
        {
            assert(bm3);
            preResult = bm3;
            bm3 = 0;
        }
    }
    if ((m_RemoveCorners || (m_PreserveShadow)) /*&& !preResult*/ && !IsWindowMaximized(target))   // We don't have to clear window corners if we already have capture with aplha-channel
    {
        bgColor = RGB(255, 0, 0);
        ShowWindow(wnd, SW_SHOWNOACTIVATE);
        ::InvalidateRect(wnd, NULL, false);
        ProcessEvents();
        ActivateWindowRepeat(target, 250);
        eng.captureRegion(&reg);
        redBgBitmap = eng.releaseCapturedBitmap();
        Bitmap* ress = 0;
        if (RemoveCorners(preResult ? preResult : bm1, redBgBitmap, &ress))
        {
            assert(ress);
            delete preResult;
            preResult = ress;
        }
    }
    if (preResult && m_PreserveShadow && !IsWindowMaximized(target))
    {
        Bitmap* shadowed = 0;
        AddBorderShadow(preResult, true, &shadowed);
        delete preResult;
        preResult = shadowed;
    }
    resultBm = preResult;
    if (target && move)
        ::SetWindowPos(target, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    DestroyWindow(wnd);
    delete bm1;
    delete bm2;
    delete bm3;
    DwmUnregisterThumbnail(thumb);
    delete redBgBitmap;
    return resultBm;
}

// TODO : fix vista maximized window capturing
bool CWindowHandlesRegion::GetImage(HDC src, Bitmap** res)
{
    if (m_hWnds.empty()) return false;
    RECT captureRect = {0, 0, 0, 0};
    if (!m_ScreenRegion.IsNull())
        m_ScreenRegion.DeleteObject();
    m_ScreenRegion.CreateRectRgnIndirect(&captureRect);
    for (size_t i = 0; i < m_hWnds.size(); i++)
    {
        CRgn newRegion = GetWindowVisibleRegion(m_hWnds[i].wnd);
        m_ScreenRegion.CombineRgn(newRegion, m_hWnds[i].Include ? RGN_OR : RGN_DIFF);
    }
    bool move = false;
    bool parentIsInList = false;
    if (m_bFromScreen)
    {
        topWindow = GetTopParent(m_hWnds[0].wnd);
        if (topWindow == m_hWnds[0].wnd)
            parentIsInList = true;
        for (size_t i = 1; i < m_hWnds.size(); i++)
        {
            HWND curTopWindow = GetTopParent(m_hWnds[i].wnd);
            if (curTopWindow == m_hWnds[i].wnd)
                parentIsInList = true;
            if (topWindow != curTopWindow)
            {
                topWindow = 0;
            }
        }
        if (topWindow)
        {
            TCHAR Buffer[MAX_PATH];
            GetClassName(topWindow, Buffer, sizeof(Buffer) / sizeof(TCHAR));
            if (lstrcmpi(Buffer, _T("Shell_TrayWnd")))
            {
                move = true;
                ::SetWindowPos(topWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                WinUtils::TimerWait(m_WindowHidingDelay);
            }
        }
    }
    CRect scr;
    GetScreenBounds(scr);
    m_ScreenRegion.OffsetRgn(-scr.left, -scr.top);
    Bitmap* resultBm = 0;
    if (m_bFromScreen && parentIsInList /*&& GetParent(topWindow)==HWND_DESKTOP */ &&  WinUtils::IsVista() &&
        IsCompositionActive() && topWindow && !(GetWindowLong(topWindow, GWL_STYLE) & WS_CHILD)
        && (m_ClearBackground || m_RemoveCorners || m_PreserveShadow))
    {
        resultBm = CaptureWithTransparencyUsingDWM();
    }
    *res = resultBm;
    if (!resultBm)
    {
        /*bool result = */CRectRegion::GetImage(src, res);
        if (topWindow && move)
            ::SetWindowPos(topWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    return resultBm != 0;
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
    for (size_t i = 0; i < m_hWnds.size(); i++)
    {
        if (m_hWnds[i].wnd == wnd)
        {
            m_hWnds.erase(m_hWnds.begin() + i);
            return;
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


CScreenCaptureEngine::CScreenCaptureEngine()
{
//    m_capturedBitmap = NULL;
    m_captureDelay = 0;
    capturedBitmapReleased_ = false;
    m_source = 0;
}

CScreenCaptureEngine::~CScreenCaptureEngine()
{
    //delete m_capturedBitmap;
}

bool CScreenCaptureEngine::captureScreen()
{
    CRect screenBounds;
    GetScreenBounds(screenBounds);
    screenBounds.OffsetRect(-screenBounds.left, -screenBounds.top);
    // int screenWidth = GetScreenWidth();//GetSystemMetrics(SM_CXSCREEN);
    // int screenHeight = GetScreenWidth();//GetSystemMetrics(SM_CYSCREEN);
    CRectRegion capturingRegion(screenBounds.left, screenBounds.top, screenBounds.right - screenBounds.left,
                                screenBounds.bottom - screenBounds.top);
    return captureRegion(&capturingRegion);
}

void CScreenCaptureEngine::setDelay(int msec)
{
    m_captureDelay = msec;
}

std::shared_ptr<Gdiplus::Bitmap> CScreenCaptureEngine::capturedBitmap()
{
    return m_capturedBitmap;
}

void CScreenCaptureEngine::setSource(HBITMAP src)
{
    m_source = src;
}

bool CScreenCaptureEngine::captureRegion(CScreenshotRegion* region)
{
//    delete m_capturedBitmap;
    //m_capturedBitmap = NULL;
    HDC srcDC;
    HDC screenDC = ::GetDC(0);
    HGDIOBJ oldBm = 0;
    if (m_source)
    {
        HDC sourceDC = CreateCompatibleDC(screenDC);
        srcDC = sourceDC;
        oldBm = SelectObject(srcDC, m_source);
    }
    else
    {
        srcDC = screenDC;
        if (m_captureDelay)
        {
            WinUtils::TimerWait(m_captureDelay);
        }
    }
    region->PrepareShooting(!(bool)(m_source != 0));
    Gdiplus::Bitmap* capturedBitmap;
    bool result =  region->GetImage(srcDC, &capturedBitmap);
    typedef release_deleter<Gdiplus::Bitmap>& releaseDeleterRef;
    //m_capturedBitmap.reset<Gdiplus::Bitmap>(capturedBitmap, std::bind(&CScreenCaptureEngine::capturedBitmapDeleteFunction, this, _1));
    capturedBitmapReleased_ = false;
    m_capturedBitmap.reset(capturedBitmap,capturedBitmapDeleter_);
    /*m_capturedBitmap.reset<Gdiplus::Bitmap,  release_deleter<Gdiplus::Bitmap>&>(capturedBitmap, std::ref(capturedBitmapDeleter_));
    m_c*apturedBitmap.reset<Gdiplus::Bitmap,  std::reference_wrapper<release_deleter<Gdiplus::Bitmap>>>(capturedBitmap, std::ref(capturedBitmapDeleter_));*
    */
    capturedBitmapDeleter_.reset_released();
    region->AfterShooting();
    if (m_source)
    {
        SelectObject(srcDC, oldBm);
        DeleteDC(srcDC);
    }
    ReleaseDC(0, screenDC);
    return result;
}

Gdiplus::Bitmap* CScreenCaptureEngine::releaseCapturedBitmap()
{
    capturedBitmapDeleter_.release();
    capturedBitmapReleased_ = true;
    Gdiplus::Bitmap*  res =  m_capturedBitmap.get();
    m_capturedBitmap.reset();
    //capturedBitmapDeleter_.reset_released();
    capturedBitmapReleased_ = false;
    return res;
}

void CScreenCaptureEngine::capturedBitmapDeleteFunction(Gdiplus::Bitmap* bm)
{
    if ( !capturedBitmapReleased_ ) {
        delete bm;
    }
}

//bool CScreenCaptureEngine::capturedBitmapReleased_ = false;
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
    if (m_curvePoints.empty()) return true;
    GraphicsPath grPath;
    std::vector<Point> points;
    std::vector<POINT> curveAvgPoints;
    average_polyline(m_curvePoints, curveAvgPoints, 29);
    for (size_t i = 0; i < curveAvgPoints.size(); i++)
    {
        points.push_back(Point(curveAvgPoints[i].x, curveAvgPoints[i].y));
    }
    if (points.empty()) return true;
    grPath.AddCurve(&points[0], points.size());
    Rect grPathRect;
    grPath.GetBounds(&grPathRect);
    int bmWidth = grPathRect.GetRight() - grPathRect.GetLeft();
    int bmHeight = grPathRect.GetBottom() - grPathRect.GetTop();
    return !(bmWidth * bmHeight);
}

bool CFreeFormRegion::GetImage(HDC src, Bitmap** res)
{
    GraphicsPath grPath;
    std::vector<Point> points;
    std::vector<POINT> curveAvgPoints;
    average_polyline(m_curvePoints, curveAvgPoints, 29);
    for (size_t i = 0; i < curveAvgPoints.size(); i++)
    {
        points.push_back(Point(curveAvgPoints[i].x, curveAvgPoints[i].y));
    }
    grPath.AddCurve(&points[0], points.size());
    Rect grPathRect;
    grPath.GetBounds(&grPathRect);
    int bmWidth = grPathRect.GetRight() - grPathRect.GetLeft();
    int bmHeight = grPathRect.GetBottom() - grPathRect.GetTop();
    CDC mem2;
    mem2.CreateCompatibleDC(src);
    CBitmap bm2;
    bm2.CreateCompatibleBitmap(src, bmWidth, bmHeight);
    HBITMAP oldBm = mem2.SelectBitmap(bm2);
    ::BitBlt(mem2, 0, 0, bmWidth, bmHeight, src, grPathRect.GetLeft(), grPathRect.GetTop(), SRCCOPY | CAPTUREBLT);
    mem2.SelectBitmap(oldBm);
    Matrix matrix(1.0f, 0.0f, 0.0f, 1.0f, REAL(-grPathRect.GetLeft()), REAL(-grPathRect.GetTop()));
    grPath.Transform(&matrix);
    Bitmap b(bm2, 0);
    SolidBrush gdipBrush(Color(255, 0, 0, 0));
    Bitmap alphaBm(bmWidth, bmHeight, PixelFormat32bppARGB);
    Graphics alphaGr(&alphaBm);
    alphaGr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
    alphaGr.SetSmoothingMode(SmoothingModeAntiAlias);
    alphaGr.FillPath(&gdipBrush, &grPath);
    Bitmap* finalbm = new Bitmap(bmWidth, bmHeight, PixelFormat32bppARGB);
    Graphics gr(finalbm);
    gr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
    gr.SetSmoothingMode(SmoothingModeAntiAlias);
    gr.DrawImage(&b, 0, 0);
    SolidBrush gdipBrush2(Color(100, 123, 0, 0));
    Pen pn(Color(255, 40, 255), 1.0f) ;
    Pen pn2(Color(40, 0, 255), 1.0f) ;
    transferOneARGBChannelFromOneBitmapToAnother(alphaBm, *finalbm, Alpha, Alpha);
    *res = finalbm;
    return true;
}

CFreeFormRegion::~CFreeFormRegion()
{
}

CActiveWindowRegion::CActiveWindowRegion()
{
}

bool CActiveWindowRegion::GetImage(HDC src, Bitmap** res)
{
    Clear();
    AddWindow(GetForegroundWindow(), true);
    return CWindowHandlesRegion::GetImage(src, res);
}
