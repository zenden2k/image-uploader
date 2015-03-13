// ImageEditorView.cpp : implementation of the CImageEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#include "ImageEditorView.h"

#include <algorithm>
#include "ImageEditor/resource.h"
#include "ImageEditor/BasicElements.h"
#include <GdiPlus.h>

#ifndef TR
#define TR(a) L##a
#endif
namespace ImageEditor {

CImageEditorView::CImageEditorView() {
	oldPoint.x = -1;
	oldPoint.y = -1;
	menuItems_[ID_PEN].toolId       = Canvas::dtPen; 
	menuItems_[ID_LINE].toolId      = Canvas::dtLine;
	menuItems_[ID_BRUSH].toolId     = Canvas::dtBrush;
	menuItems_[ID_RECTANGLE].toolId = Canvas::dtRectangle;
	menuItems_[ID_CROP].toolId = Canvas::dtRectangle;
}

BOOL CImageEditorView::PreTranslateMessage(MSG* /*pMsg*/) {
	return FALSE;
}

LRESULT CImageEditorView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	CPaintDC dc(m_hWnd);
	CRgn rgn;
	rgn.CreateRectRgn( 0, 0, 0, 0 );
	GetClipRgn( dc, rgn);
	RECT rct;
	rgn.GetRgnBox( &rct );
	Gdiplus::Graphics gr(dc);
	if ( canvas_ ) {
		canvas_->render( &gr, dc.m_ps.rcPaint );
	}

	return 0;
}


void CImageEditorView::setCanvas(ImageEditor::Canvas *canvas) {
	canvas_ = canvas;
	if ( canvas ) {
		canvas_->setSize( 1280, 720 );
		canvas_->setCallback( this );
		canvas_->setDrawingToolType(Canvas::dtRectangle);
	}
}

LRESULT CImageEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);

	canvas_->mouseMove( cx, cy, wParam );
	return 0;
}

LRESULT CImageEditorView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	SetCapture();
	canvas_->mouseDown( 0, cx, cy );
	return 0;
}

LRESULT CImageEditorView::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	canvas_->mouseUp( 0, cx, cy );
	ReleaseCapture();
	return 0;
}

void CImageEditorView::updateView( Canvas* canvas, const CRgn& region ) {
	InvalidateRgn( region );
	RECT rc;
	region.GetRgnBox( &rc );
	//InvalidateRect( &boundingRect );
}

LRESULT CImageEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {

	return 1;
}

LRESULT CImageEditorView::OnContextMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	HWND 	hwnd = (HWND) wParam;  
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
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}
	
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_UNDO, TR("Отменить"));
	menu.AppendMenu(MF_STRING, ID_PEN, TR("Карандаш"));
	menu.AppendMenu(MF_STRING, ID_BRUSH, TR("Кисть"));
	menu.AppendMenu(MF_STRING, ID_LINE, TR("Линия"));
	menu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("Прямоугольник"));
	menu.AppendMenu(MF_STRING, ID_TEXT, TR("Добавить текст"));
	menu.AppendMenu(MF_STRING, ID_CROP, TR("Обрезка"));

	//menu.SetMenuDefaultItem(0, true);
	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);

	return 0;
}

LRESULT CImageEditorView::OnMenuItemClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int toolId = menuItems_[wID].toolId;
	canvas_->setDrawingToolType( static_cast<Canvas::DrawingToolType>( toolId ) );
	return 0;
}

LRESULT CImageEditorView::OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	canvas_->undo();
	return 0;
}


LRESULT CImageEditorView::OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	
	canvas_->setDrawingToolType( Canvas::dtText );
	return 0;
}

LRESULT CImageEditorView::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	SetCursor(getCachedCursor(canvas_->getCursor()));
	return 0;
}


HCURSOR CImageEditorView::getCachedCursor(Canvas::CursorType cursorType)
{
	HCURSOR cur = cursorCache_[cursorType];
	if ( cur ) {
		return cur;
	}
	LPCTSTR lpCursorName = 0;
	switch( cursorType ) {
		case Canvas::ctEdit:
			lpCursorName = IDC_IBEAM;
			break;
		case Canvas::ctResizeVertical:
			lpCursorName = IDC_SIZENS;
			break;
		case Canvas::ctResizeHorizontal:
			lpCursorName = IDC_SIZEWE;
			break;
		case Canvas::ctResizeDiagonalMain:
			lpCursorName = IDC_SIZENWSE;
			break;
		case Canvas::ctResizeDiagonalAnti:
			lpCursorName = IDC_SIZENESW;
			break;
		case Canvas::ctCross:
			lpCursorName = IDC_CROSS;
			break;
		default:
			lpCursorName = IDC_ARROW;
	}
	cur = LoadCursor(0, lpCursorName);
	cursorCache_[cursorType] = cur;
	return cur;
}

}
