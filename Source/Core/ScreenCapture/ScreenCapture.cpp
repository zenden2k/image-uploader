#include "ScreenCapture.h"
#include <math.h>
#include <QPoint>

#include <deque>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

#include <QEventLoop>
#include <QTimer>

#ifdef _WIN32

#include <windows.h>
typedef HRESULT (WINAPI *DwmGetWindowAttribute_Func)(HWND, DWORD, PVOID, DWORD);
typedef HRESULT (WINAPI *DwmIsCompositionEnabled_Func)(BOOL*);

QRegion QRegionFromHRGN(HRGN winRegion)
{
    QRegion result;
    LPRGNDATA data;
    DWORD size = GetRegionData(winRegion, 0, 0);
    if(!size) return result;
    data = (LPRGNDATA)new char [size];
    data->rdh.dwSize = sizeof(RGNDATAHEADER);
    GetRegionData(winRegion, size, data);
    RECT *rects = (RECT*) data->Buffer;
    for(size_t i = 0; i < data->rdh.nCount; i++)
    {
        RECT rc = rects[i];
        QRect rct = QRect(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top);
        result|= rct;
    }
    delete[] data;
    return result;
}

BOOL MyGetWindowRect(HWND hWnd, RECT *res)
{
/*    if(!IsVista())
    {
        return GetWindowRect(hWnd, res);
    }*/
    /*static HMODULE DllModule =  LoadLibrary(L"dwmapi.dll");
    if(DllModule)
    {
        DwmIsCompositionEnabled_Func IsCompEnabledFunc = (DwmIsCompositionEnabled_Func) GetProcAddress(DllModule, "DwmIsCompositionEnabled");
        if(IsCompEnabledFunc)
        {
            BOOL isEnabled = false;
            if(S_OK == IsCompEnabledFunc( &isEnabled))
                if(isEnabled)
                {
                    DwmGetWindowAttribute_Func Func = (DwmGetWindowAttribute_Func) GetProcAddress(DllModule, "DwmGetWindowAttribute");
                    if(Func)
                    {
                        if(S_OK == Func( hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, res, sizeof(RECT)))
                            return TRUE;
                    }
            }
        }    
    }*/
    return GetWindowRect(hWnd, res);
}
HWND GetTopParent(HWND wnd)
{
    //HWND res;
    if(GetWindowLong(wnd,GWL_STYLE) & WS_CHILD)
    {

        wnd = ::GetParent(wnd);
    }
    return wnd;
}

/*
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

        }

        rect = result;


    return true;
}*/

#endif

namespace ZDesktopTools
{
#ifdef _WIN32
    HRGN Win_GetWindowRegion(HWND wnd)
    {
        RECT WndRect;
        ::MyGetWindowRect(wnd, &WndRect );
        HRGN WindowRgn = CreateRectRgnIndirect(&WndRect);

        if(::GetWindowRgn(wnd, WindowRgn) != ERROR)
        {
            //WindowRegion.GetRgnBox( &WindowRect);
            OffsetRgn(WindowRgn, WndRect.left, WndRect.top);
        }
        return WindowRgn;
    }

    HRGN Win_GetWindowVisibleRegion(HWND wnd)
    {
        HRGN winReg;
        RECT result;

        if(!(GetWindowLong(wnd,GWL_STYLE) & WS_CHILD))
        {
            winReg = Win_GetWindowRegion(wnd);
            return winReg;
        }
        MyGetWindowRect(wnd, &result);
        while(GetWindowLong(wnd,GWL_STYLE) & WS_CHILD)
        {
            wnd = GetParent(wnd);
            //HRGN parentRgn;
            RECT rc;
            if(GetClientRect(wnd,&rc))
            {
                ::MapWindowPoints(wnd,0,(POINT*)&rc,2);
            }
            IntersectRect(&result, &result, &rc);
        }

        winReg = CreateRectRgnIndirect(&result);
        return winReg;
    }


#endif
    int GetActiveWindow()
    {
#ifdef _WIN32
        return (int) GetForegroundWindow();
#endif
    }

    QRegion GetWindowVisibleRegion(int wnd)
    {
        #ifdef _WIN32
            HRGN rgn = /*Win_GetWindowVisibleRegion*/Win_GetWindowRegion((HWND)wnd);
            QRegion result =  QRegionFromHRGN(rgn);
            DeleteObject(rgn);
            return result;
        #endif
    }
};
/*int GetScreenWidth()
{
    HDC dc = GetDC(0);
    int result =  GetDeviceCaps(dc, DESKTOPHORZRES);
    ReleaseDC(0, dc);
    return result;
    /HWND wnd = GetDesktopWindow();
    RECT rect;
    GetWindowRect(wnd, &rect);*
    //return rect.right;
}

int GetScreenHeight()
{
    HDC dc = GetDC(0);
    int result = GetDeviceCaps(dc, DESKTOPVERTRES    );
    ReleaseDC(0, dc);
    return result;
    /HWND wnd = GetDesktopWindow();
    RECT rect;
    GetWindowRect(wnd, &rect);*
    //return rect.bottom;
}
*/
void average_polyline(std::vector<QPoint>& path, std::vector<QPoint>& path2, unsigned n);

enum ChannelARGB
{
    Blue = 0,
    Green = 1,
    Red = 2,
    Alpha = 3
};

// hack for stupid GDIplus
/*void transferOneARGBChannelFromOneBitmapToAnother(Bitmap& source, Bitmap& dest, ChannelARGB sourceChannel, ChannelARGB destChannel )
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
*/
void average_polyline(std::vector<QPoint>& path, std::vector<QPoint>& path2, unsigned n)
{
    if(path.size() > 2)
    {
        std::deque<QPoint> tmp;
        unsigned i, j;
        for(j = 0; j < path.size(); j++)
        {
            tmp.push_back(path[j]);
        }

        for(i = 0; i < n; i++)
        {
            path2.clear();
            for(j = 0; j < tmp.size(); j++)
            {
                QPoint p(tmp[j].x(), tmp[j].y());
                int d = 1;
                if(j) 
                {
                    p.rx() += tmp[j-1].x();
                    p.ry() += tmp[j-1].y();
                    ++d;
                }
                if(j + 1 < tmp.size()) 
                {
                    p.rx() += tmp[j+1].x();
                    p.ry() += tmp[j+1].y();
                    ++d;
                }
                QPoint newP(p.x() / d, p.y() / d);
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



CRectRegion::CRectRegion()
{

}

CRectRegion::~CRectRegion()
{
}

CRectRegion::CRectRegion(int x, int y, int width, int height)
{
    /*if(!m_ScreenRegion.isEmpty())
        m_ScreenRegion=QRegion();*/
    m_ScreenRegion = QRegion(x, y, width, height);
}

CRectRegion::CRectRegion(QRegion region)
{
    /*if(!m_ScreenRegion.IsNull())
        m_ScreenRegion.DeleteObject();*/
    m_ScreenRegion = region;
}

bool CRectRegion::IsEmpty()
{
    return m_ScreenRegion.boundingRect().isEmpty();
}

QRect CRectRegion::getBoundingRect()
{
    return m_ScreenRegion.boundingRect();
}

bool CRectRegion::GetImage(QPixmap * src, QPixmap ** res)
{
    if(m_ScreenRegion.rectCount() == 1)
    {
        QPixmap *result  = new QPixmap();
        *result = *src;
        *res = result;
        return true;
    }

    QPixmap *result;
    result = new QPixmap(m_ScreenRegion.boundingRect().size());
    result->fill(QColor(0,0,0,0));
    QPainter p(result);

    p.setClipRegion(m_ScreenRegion.translated(-m_ScreenRegion.boundingRect().topLeft()));
    p.drawPixmap(0,0, *src);
    *res = result;
    return true;
    //src->copy()
    /*RECT regionBoundingRect;


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

    *res = resultBm;*/
    return true;
}


CWindowHandlesRegion::CWindowHandlesRegion()
{
    topWindow = 0;
    m_WindowHidingDelay = 0;
}

CWindowHandlesRegion::CWindowHandlesRegion(int wnd)
{
    CWindowHandlesRegionItem newItem;
    newItem.Include= true;
    newItem.wnd = wnd;
    m_Windows.push_back(newItem);
}

void CWindowHandlesRegion::SetWindowHidingDelay(int delay)
{
    m_WindowHidingDelay = delay;
}

bool CWindowHandlesRegion::GetImage(QPixmap * src, QPixmap ** res)
{
    if(m_Windows.empty()) return false;
    /*RECT captureRect={0,0,0,0};
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
    }*/


    /*CRect scr;
    GetScreenBounds(scr);
    m_ScreenRegion.OffsetRgn(-scr.left,-scr.top);*/
    bool result = CRectRegion::GetImage(src, res);

    /*if(topWindow && move)
        ::SetWindowPos(topWindow,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);*/
    return result;
}

CWindowHandlesRegion::~CWindowHandlesRegion()
{
}

bool CWindowHandlesRegion::PrepareShooting(bool fromScreen)
{
    m_ScreenRegion = QRegion();
    for(size_t i=0; i<m_Windows.size(); i++)
    {
        QRegion newRegion = ZDesktopTools::GetWindowVisibleRegion(m_Windows[i].wnd);
        //if(m_Windows[i].Include )
        m_ScreenRegion|=newRegion;
        //m_ScreenRegion.CombineRgn(newRegion, m_hWnds[i].Include ? RGN_OR: RGN_DIFF);
    }
    CRectRegion::PrepareShooting(fromScreen);
    return true;
}

void CWindowHandlesRegion::AddWindow(int wnd, bool Include)
{
    CWindowHandlesRegionItem newItem;
    newItem.wnd = wnd;
    newItem.Include = Include;
    RemoveWindow(wnd);
    m_Windows.push_back(newItem);
}

void CWindowHandlesRegion::RemoveWindow(int wnd)
{
    for(size_t i=0; i< m_Windows.size(); i++)
    {
        if(m_Windows[i].wnd == wnd)
        {
            m_Windows.erase(m_Windows.begin() + i); return;
        }
    }
}

bool CWindowHandlesRegion::IsEmpty()
{
    return m_Windows.empty();
}
void CWindowHandlesRegion::Clear()
{
    m_Windows.clear();
}



void TimerWait(int Delay)
{
    QEventLoop loop;
    QTimer t;
    t.connect(&t, SIGNAL(timeout()), &loop, SLOT(quit()));
    t.start(Delay);

    loop.exec();
    /*HANDLE hTimer = CreateWaitableTimer(0, TRUE, 0);
    LARGE_INTEGER interval;

    interval.QuadPart = -Delay * 10000;
    SetWaitableTimer(hTimer,&interval,0,0,0,0);
    MsgWaitForSingleObject(hTimer, INFINITE);
    CloseHandle(hTimer);*/
}
CScreenCaptureEngine::CScreenCaptureEngine()
{
    m_capturedBitmap = 0;
    m_captureDelay = 0;
//    m_source = 0;
}

CScreenCaptureEngine::~CScreenCaptureEngine()
{
    delete m_capturedBitmap;
}

bool CScreenCaptureEngine::captureScreen()
{
    /*CRect screenBounds;
    GetScreenBounds(screenBounds);
    screenBounds.OffsetRect(-screenBounds.left,-screenBounds.top);
    //int screenWidth = GetScreenWidth();//GetSystemMetrics(SM_CXSCREEN);
    //int screenHeight = GetScreenWidth();//GetSystemMetrics(SM_CYSCREEN);
    */
    QRect screenBounds = QApplication::desktop()->rect();
CRectRegion capturingRegion(screenBounds.left(), screenBounds.top(), screenBounds.width(), screenBounds.height());
    return captureRegion(&capturingRegion);
}

void CScreenCaptureEngine::setDelay(int msec)
{
    m_captureDelay = msec;
}

QPixmap* CScreenCaptureEngine::capturedBitmap()
{
    return m_capturedBitmap;
}

void CScreenCaptureEngine::setSource(QPixmap src)
{
    m_source = src;
}
bool CScreenCaptureEngine::captureRegion(CScreenshotRegion* region)
{
    delete m_capturedBitmap;
    if(!region) return false;

    region->PrepareShooting(true);

    QRect grabRect = region->getBoundingRect();
    qDebug()<<grabRect;
    bool result = false;
    if(m_source.isNull())
    {
        if(m_captureDelay)
        {
            TimerWait(m_captureDelay);
        }
        QPixmap screenCapture = QPixmap::grabWindow(QApplication::desktop()->winId(), grabRect.x(), grabRect.y(), grabRect.width(), grabRect.height());
         result =  region->GetImage(&screenCapture, &m_capturedBitmap);
    }
    else
    {
        QPixmap screenCapture = m_source.copy(grabRect.x(), grabRect.y(), grabRect.width(), grabRect.height());
         result =  region->GetImage(&screenCapture, &m_capturedBitmap);
     }
    return result;
    /*delete m_capturedBitmap;
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
    return result;*/
}

CFreeFormRegion::CFreeFormRegion()
{
}

void CFreeFormRegion::AddPoint(QPoint point)
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
    /*GraphicsPath grPath;
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

    return !(bmWidth*bmHeight);*/
    return false;
}

bool CFreeFormRegion::GetImage(QPixmap * src, QPixmap ** res)
{
    /*GraphicsPath grPath;
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
    *res = finalbm;*/
    return true;
}

CFreeFormRegion::~CFreeFormRegion()
{
}

CActiveWindowRegion::CActiveWindowRegion()
{

}

bool CActiveWindowRegion::PrepareShooting(bool fromScreen)
{
    Clear();
    AddWindow(ZDesktopTools::GetActiveWindow(), true);
    return CWindowHandlesRegion::PrepareShooting(fromScreen);
}

bool CActiveWindowRegion::GetImage(QPixmap * src, QPixmap ** res)
{
    return CWindowHandlesRegion::GetImage(src, res);
}
