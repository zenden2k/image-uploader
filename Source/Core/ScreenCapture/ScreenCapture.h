#ifndef IU_CORE_SCREENCAPTURE_SCREENCAPTURE_H
#define IU_CORE_SCREENCAPTURE_SCREENCAPTURE_H

#include <QPixmap>
#include <QPaintDevice>

#include "CommonTypes.h"
#include "Core/Utils/CoreTypes.h"

/*bool GetScreenBounds(RECT &rect);
HRGN GetWindowVisibleRegion(int windo wnd);
*/
void TimerWait(int Delay);
enum CaptureMode {cmFullScreen, cmActiveWindow, cmRectangles, cmFreeform, cmWindowHandles };

class CScreenshotRegion
{
public:
    virtual bool GetImage(QPixmap * src, QPixmap ** res)=0;

    virtual ~CScreenshotRegion() {
        
    };
    virtual bool PrepareShooting(bool fromScreen) {
        m_bFromScreen = fromScreen;
        return true;
    }

    virtual void AfterShooting(){
    }

    virtual bool IsEmpty() const {
        return false;
    }

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
    bool GetImage(QPixmap * src, QPixmap ** res) override;
    QRect getBoundingRect() override;
    bool IsEmpty() const override;
    ~CRectRegion();
protected:
    QRegion m_ScreenRegion;
    DISALLOW_COPY_AND_ASSIGN(CRectRegion);
};

class CWindowHandlesRegion: public CRectRegion
{
public:
    CWindowHandlesRegion();
    explicit CWindowHandlesRegion(WId windowId);
    bool PrepareShooting(bool fromScreen) override;
    void AddWindow(WId windowId, bool Include);
    void RemoveWindow(WId windowId);
    void Clear();
    void SetWindowHidingDelay(int delay /* msecs */ );
    bool GetImage(QPixmap * src, QPixmap ** res) override;
    bool IsEmpty() const override;
    ~CWindowHandlesRegion();

    struct CWindowHandlesRegionItem
    {
        WId wnd;
        bool Include;
    };

protected:
    int topWindow;
    int m_WindowHidingDelay;
    std::vector<CWindowHandlesRegionItem> m_Windows;
    DISALLOW_COPY_AND_ASSIGN(CWindowHandlesRegion);
};

class CActiveWindowRegion: public CWindowHandlesRegion
{
public:
    CActiveWindowRegion();
    bool GetImage(QPixmap * src, QPixmap ** res) override;
    bool PrepareShooting(bool fromScreen) override;
    DISALLOW_COPY_AND_ASSIGN(CActiveWindowRegion);
};

class CFreeFormRegion: public CRectRegion
{
public:
    CFreeFormRegion();
    void AddPoint(QPoint point);
    void Clear();
    bool IsEmpty() const override;
    bool GetImage(QPixmap * src, QPixmap ** res) override;
    ~CFreeFormRegion();
protected:
    std::vector<QPoint> m_curvePoints;
    DISALLOW_COPY_AND_ASSIGN(CFreeFormRegion);
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
    QPixmap * capturedBitmap() const;
private:
    int m_captureDelay;
    QPixmap  *m_capturedBitmap;
    QPixmap m_source;
    DISALLOW_COPY_AND_ASSIGN(CScreenCaptureEngine);
};

#endif
