// ImageEditorView.cpp : implementation of the CImageEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#include "ImageEditorView.h"

#include "3rdpart/GdiplusH.h"
#include "Gui/GuiTools.h"
#include "../MovableElements.h"
#include "resource.h"

namespace ImageEditor {

CImageEditorView::CImageEditorView():
        dpiScaleX_(1.0f),
        dpiScaleY_(1.0f)  {
    oldPoint.x = -1;
    oldPoint.y = -1;
    mouseDown_ = false;
    canvas_ = nullptr;
}

CImageEditorView::~CImageEditorView()
{
}

BOOL CImageEditorView::PreTranslateMessage(MSG* /*pMsg*/) {
    return FALSE;
}

LRESULT CImageEditorView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
    CPaintDC dc(m_hWnd);
    RECT clientRect;
    GetClientRect(&clientRect);
    SIZE size = {clientRect.right, clientRect.bottom};

    POINT pt;
    GetScrollOffset(pt);
    if ( canvas_ ) {
        RECT updateRect = dc.m_ps.rcPaint;
        canvas_->render( dc, updateRect, pt,  size);

        int rightMargin = canvas_->getWidth() - pt.x;
        int bottomMargin = canvas_->getHeigth()-pt.y;
        RECT rightRect = {rightMargin, 0, clientRect.right,bottomMargin};
        dc.FillRect(&rightRect, backgroundBrush_);
        RECT bottomRect = {0, bottomMargin, clientRect.right, clientRect.bottom};
        dc.FillRect(&bottomRect, backgroundBrush_);
    }
    return 0;
}


LRESULT CImageEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    backgroundBrush_.CreateSolidBrush(GetSysColor(COLOR_APPWORKSPACE));
    CClientDC hdc(m_hWnd);
    dpiScaleX_ = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    dpiScaleY_ = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    return 0;
}

void CImageEditorView::setCanvas(ImageEditor::Canvas *canvas) {
    canvas_ = canvas;
    if ( canvas ) {
        SIZE sz = {canvas_->getWidth(), canvas_->getHeigth()};
        SetScrollOffset(0, 0);
        SetScrollSize(sz);

        canvas->setDpi(dpiScaleX_, dpiScaleY_);
        canvas_->setCallback( this );
        canvas_->setDrawingToolType(DrawingToolType::dtRectangle);
    }
}

LRESULT CImageEditorView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
    int cx = GET_X_LPARAM(lParam); 
    int cy = GET_Y_LPARAM(lParam);
    POINT ptScroll;
    GetScrollOffset(ptScroll);
    cx += ptScroll.x;
    cy += ptScroll.y;
    POINT pt = {cx, cy};
    RECT canvasRect = {0,0, canvas_->getWidth(), canvas_->getHeigth()};
    if ( mouseDown_ || PtInRect(&canvasRect, pt) ){
        canvas_->mouseMove( cx, cy, wParam );
    }
    
    return 0;
}

LRESULT CImageEditorView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
    int cx =   GET_X_LPARAM(lParam); 
    int cy =   GET_Y_LPARAM(lParam); 
    
    if (GetFocus() != m_hWnd)
    {
        SetFocus();
    }

    POINT ptScroll;
    GetScrollOffset(ptScroll);
    cx += ptScroll.x;
    cy += ptScroll.y;
    POINT pt = {cx, cy};
    RECT canvasRect = {0,0, canvas_->getWidth(), canvas_->getHeigth()};
    if ( PtInRect(&canvasRect, pt) ){
        mouseDown_ = true;
        SetCapture();
        canvas_->mouseDown( 0, cx, cy );
    }
    return 0;
}

LRESULT CImageEditorView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
    if ( !mouseDown_ ) {
        return 0;
    }
    int cx =   GET_X_LPARAM(lParam); 
    int cy =   GET_Y_LPARAM(lParam);
    POINT ptScroll;
    GetScrollOffset(ptScroll);
    cx += ptScroll.x;
    cy += ptScroll.y;
    POINT pt = {cx, cy};
    RECT canvasRect = {0,0, canvas_->getWidth(), canvas_->getHeigth()};
    /*if ( PtInRect(&canvasRect, pt) )*/{
        canvas_->mouseUp( 0, cx, cy );
    }
    ReleaseCapture();
    mouseDown_ = false;
    
    return 0;
}

LRESULT CImageEditorView::OnRButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    int cx =   GET_X_LPARAM(lParam); 
    int cy =   GET_Y_LPARAM(lParam);
    POINT ptScroll;
    GetScrollOffset(ptScroll);
    cx += ptScroll.x;
    cy += ptScroll.y;
    POINT pt = {cx, cy};
    RECT canvasRect = {0,0, canvas_->getWidth(), canvas_->getHeigth()};
    /*if ( PtInRect(&canvasRect, pt) )*/{
        canvas_->mouseUp( 1, cx, cy );
        ReleaseCapture();
    }
    return 0;
}

LRESULT CImageEditorView::OnLButtonDblClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    int cx = GET_X_LPARAM(lParam);
    int cy = GET_Y_LPARAM(lParam);
    POINT ptScroll;
    GetScrollOffset(ptScroll);
    cx += ptScroll.x;
    cy += ptScroll.y;
    POINT pt = {cx, cy};
    RECT canvasRect = {0,0, canvas_->getWidth(), canvas_->getHeigth()};
    if ( PtInRect(&canvasRect, pt) ){
        canvas_->mouseDoubleClick( 0, cx, cy );
    }
    return 0;
}

void CImageEditorView::updateView(Canvas* canvas, Gdiplus::Rect rect, bool fullRender)
{
    RECT rc {};
    if (fullRender) {
        GetClientRect(&rc);
    } else {
        POINT pt;
        GetScrollOffset(pt);
        rect.Offset(-pt.x, -pt.y);

        rc = { rect.X, rect.Y, rect.GetRight(), rect.GetBottom() };
    }
    
    //RECT fullRect = { 0,0, canvas_->getWidth(), canvas_->getHeigth()};
    //GetClientRect(&clientRect);
    InvalidateRect(&rc);
}

LRESULT CImageEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
    bHandled = true;
    return 1;
}

LRESULT CImageEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
    /*HWND     hwnd = (HWND) wParam;  
    POINT ClientPoint, ScreenPoint;

    if(lParam == -1) 
    {
        ClientPoint.x = 0;
        ClientPoint.y = 0;
        ScreenPoint = ClientPoint;
        ::ClientToScreen(hwnd, &ScreenPoint);
    }
    else
    {
        ScreenPoint.x = GET_X_LPARAM(lParam); 
        ScreenPoint.y = GET_Y_LPARAM(lParam); 
        ClientPoint = ScreenPoint;
        ::ScreenToClient(hwnd, &ClientPoint);
    }*/
    return 0;
}


LRESULT CImageEditorView::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    RECT clientRect;
    GetClientRect(&clientRect);
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    if ( WindowFromPoint(cursorPos ) != m_hWnd ) {
        //SetCursor(getCachedCursor(ctDefault));
        return 0;
    }
    ScreenToClient(&cursorPos);

    POINT ptScroll;
    GetScrollOffset(ptScroll);

    POINT pt = {cursorPos.x + ptScroll.x, cursorPos.y + ptScroll.y };

    RECT canvasRect = {0,0, canvas_->getWidth(), canvas_->getHeigth()};

    if ( PtInRect(&clientRect, cursorPos ) && PtInRect(&canvasRect, pt) ) {
        SetCursor(getCachedCursor(canvas_->getCursor()));
    } else {
        SetCursor(getCachedCursor(CursorType::ctDefault));
    }
    return 0;
}


LRESULT CImageEditorView::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    ::SendMessage(GetParent(), uMsg, wParam, lParam);
    return 0;
}

LRESULT CImageEditorView::OnKeyUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
    ::SendMessage(GetParent(), uMsg, wParam, lParam);
    return 0;
}

LRESULT CImageEditorView::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    Invalidate(true);
    return 0;
}

HCURSOR CImageEditorView::getCachedCursor(CursorType cursorType)
{
    HCURSOR cur = cursorCache_[cursorType];
    int penSize = canvas_->getPenSize();
    if ( cur ) {
        return cur;
    }
    LPCTSTR lpCursorName = 0;
    switch( cursorType ) {
        case CursorType::ctEdit:
            lpCursorName = IDC_IBEAM;
            break;
        case CursorType::ctResizeVertical:
            lpCursorName = IDC_SIZENS;
            break;
        case CursorType::ctResizeHorizontal:
            lpCursorName = IDC_SIZEWE;
            break;
        case CursorType::ctResizeDiagonalMain:
            lpCursorName = IDC_SIZENWSE;
            break;
        case CursorType::ctResizeDiagonalAnti:
            lpCursorName = IDC_SIZENESW;
            break;
        
        case CursorType::ctMove:
            lpCursorName = IDC_SIZEALL;
            break;
        case CursorType::ctCross:
            cur = LoadCursor(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDC_CROSSHAIRCURSOR));
            cursorCache_[cursorType] = cur;
            return cur;
        case CursorType::ctColorPicker:
            cur = LoadCursor(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDC_COLORPICKERCURSOR));
            cursorCache_[cursorType] = cur;
            return cur;
        case CursorType::ctBrush:
            if ( brushCursorCache_[penSize] ) {
                return brushCursorCache_[penSize] ;
            }
            cur = createBrushCursor(penSize);
            brushCursorCache_[penSize] = cur;
            return cur;
        default:
            lpCursorName = IDC_ARROW;
    }
    cur = LoadCursor(0, lpCursorName);
    cursorCache_[cursorType] = cur;
    return cur;
}
HICON CImageEditorView::createBrushCursor(int size)
{
    if ( size < 3) {
        size = 3;
    }
    using namespace Gdiplus;
    size+=1;
    Bitmap bm(size,size, PixelFormat32bppARGB);
    Graphics gr(&bm);
    gr.SetSmoothingMode(SmoothingModeAntiAlias);
    //gr.SetPixelOffsetMode(PixelOffsetModeHalf);

    Pen pen2(Color(100, 100, 100), 3.0f);
    gr.DrawEllipse(&pen2, 1, 1, size - 3, size - 3);

    Pen pen(Color(255, 255, 255), 1.0f);
    gr.DrawEllipse(&pen,1,1,size-3,size-3);
    
    HICON res;
    bm.GetHICON(&res);
    return res;
}

void CImageEditorView::canvasSizeChanged() {
    SIZE sz = { canvas_->getWidth(), canvas_->getHeigth() };
    SetScrollOffset(0, 0);
    SetScrollSize(sz);
}

}
