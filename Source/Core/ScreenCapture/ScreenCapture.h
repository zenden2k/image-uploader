#ifndef IU_SCREENCAPTURE_H
#define IU_SCREENCAPTURE_H
#include "CommonTypes.h"
#include <QPixmap>
#include <QPaintDevice>

/*bool GetScreenBounds(RECT &rect);
HRGN GetWindowVisibleRegion(int windo wnd);
*/
void TimerWait(int Delay);
enum CaptureMode {cmFullScreen, cmActiveWindow, cmRectangles, cmFreeform, cmWindowHandles };


class CScreenshotRegion
{
public:
    virtual bool GetImage(QPixmap * src, QPixmap ** res)=0;
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
    virtual QRect getBoundingRect()=0;
protected:
    bool m_bFromScreen;
};

class CRectRegion: public CScreenshotRegion
{
public:
    CRectRegion();
    CRectRegion(int x, int y, int width, int height);
    CRectRegion(QRegion region);
    virtual bool GetImage(QPixmap * src, QPixmap ** res);
     QRect getBoundingRect();
    bool IsEmpty();
    ~CRectRegion();
protected:
    QRegion m_ScreenRegion;
};

struct CWindowHandlesRegionItem
{
    int wnd;
    bool Include;
};

class CWindowHandlesRegion: public CRectRegion
{
public:

    CWindowHandlesRegion();
    CWindowHandlesRegion(int windowId);
    bool PrepareShooting(bool fromScreen);
    void AddWindow(int windowId, bool Include);
    void RemoveWindow(int windowId);
    void Clear();
    void SetWindowHidingDelay(int delay);
    virtual bool GetImage(QPixmap * src, QPixmap ** res);
    bool IsEmpty();
    ~CWindowHandlesRegion();
protected:
    int topWindow;
    int m_WindowHidingDelay;
    std::vector<CWindowHandlesRegionItem> m_Windows;
};

class CActiveWindowRegion: public CWindowHandlesRegion
{
public:
    CActiveWindowRegion();
    virtual bool GetImage(QPixmap * src, QPixmap ** res);
    bool PrepareShooting(bool fromScreen);
};


class CFreeFormRegion: public CRectRegion
{
public:
    CFreeFormRegion();
    void AddPoint(QPoint point);
    void Clear();
    bool IsEmpty();
    virtual bool GetImage(QPixmap * src, QPixmap ** res);
    ~CFreeFormRegion();
protected:
    std::vector<QPoint> m_curvePoints;
};



class CScreenCaptureEngine
{
public:
    CScreenCaptureEngine();
    ~CScreenCaptureEngine();
    bool captureScreen();
    void setSource(QPixmap src);
    bool captureRegion(CScreenshotRegion* region);
    void setDelay(int msec);
    QPixmap * capturedBitmap();
private:
    int m_captureDelay;
    QPixmap  *m_capturedBitmap;
    QPixmap m_source;
};

#endif
