// ImageEditorView.cpp : implementation of the CImageEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#include "ImageEditorView.h"

#include <algorithm>

#include <GdiPlus.h>
#include <Gui/GuiTools.h>
#include <Core/Logging.h>
#include <Core/Images/Utils.h>
#include "../MovableElements.h"
#include "resource.h"

#ifndef TR
#define TR(a) L##a
#endif
namespace ImageEditor {

CImageEditorView::CImageEditorView()  {
	oldPoint.x = -1;
	oldPoint.y = -1;
}

BOOL CImageEditorView::PreTranslateMessage(MSG* /*pMsg*/) {
	return FALSE;
}

LRESULT CImageEditorView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	//return 0;
	CPaintDC dc(m_hWnd);
	/*CRgn rgn;
	rgn.CreateRectRgn( 0, 0, 0, 0 );
	/*GetClipRgn( dc, rgn);
	RECT rct;
	rgn.GetRgnBox( &rct );*/
	RECT clientRect;
	GetClientRect(&clientRect);
	SIZE size = {clientRect.right, clientRect.bottom};
	Gdiplus::Graphics gr(dc);
	POINT pt;
	GetScrollOffset(pt);
	if ( canvas_ ) {
		RECT updateRect = dc.m_ps.rcPaint;
		/*updateRect.left += pt.x;
		updateRect.top += pt.y;
		updateRect.bottom += pt.y;
		updateRect.right += pt.x;*/
		canvas_->render( &gr, updateRect, pt,  size);
	}
	CBrush br;
	br.CreateSolidBrush(RGB(255,255,255));
	int rightMargin = canvas_->getWidth() - pt.x;
	int bottomMargin = canvas_->getHeigth()-pt.y;
	RECT rightRect = {rightMargin, 0, clientRect.right,bottomMargin};
	dc.FillRect(&rightRect, br);
	RECT bottomRect = {0, bottomMargin, clientRect.right, clientRect.bottom};
	dc.FillRect(&bottomRect, br);
	//horizontalToolbar_.Invalidate(TRUE);

	return 0;
}


LRESULT CImageEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

void CImageEditorView::setCanvas(ImageEditor::Canvas *canvas) {
	canvas_ = canvas;
	if ( canvas ) {
		SIZE sz = {canvas_->getWidth(), canvas_->getHeigth()};
		SetScrollOffset(0, 0);
		SetScrollSize(sz);
		canvas_->setCallback( this );
		canvas_->setDrawingToolType(Canvas::dtRectangle);
	}
}

LRESULT CImageEditorView::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = GET_X_LPARAM(lParam); 
	int cy =  GET_Y_LPARAM(lParam);
	POINT ptScroll;
	GetScrollOffset(ptScroll);
	cx += ptScroll.x;
	cy += ptScroll.y;
	POINT pt = {cx, cy};
	//LOG(INFO) <<"x=" << cx <<"y=" << cy;
	/*TextElement *textElement = < canvas_->getCurrentlyEditedTextElement();
	if ( textElement ) {
		int elX = textElement->getX();
		int elY = textElement->getY();
		RECT rc  = {elX, elY, elX + textElement->getWidth(), elY + textElement->getHeight()};
		if ( textElement && PtInRect(&rc, pt) ) {
			InputBoxControl* inputBoxControl = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
			inputBoxControl->SendMessage(uMsg, MAKEWPARAM(cx - elX, cy - elY), lParam);
		}
		return 0;
	}*/
	/*RECT toolBarRect;
	horizontalToolbar_.GetClientRect(&toolBarRect);
	horizontalToolbar_.ClientToScreen(&toolBarRect);
	ClientToScreen(&pt);*/
	/*if ( pt.x >= toolBarRect.left && pt.x <= toolBarRect.right && pt.y >= toolBarRect.top && pt.y <= toolBarRect.bottom ) {
		return 0;
	}*/
//	HWND wnd =  WindowFromPoint(pt);
	/*if ( wnd == m_hWnd )*/ {
		canvas_->mouseMove( cx, cy, wParam );
	}
	return 0;
}

LRESULT CImageEditorView::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx =   GET_X_LPARAM(lParam); 
	int cy =   GET_Y_LPARAM(lParam); 
	POINT ptScroll;
	GetScrollOffset(ptScroll);
	cx += ptScroll.x;
	cy += ptScroll.y;
	POINT pt = {cx, cy};
	/*TextElement *textElement = canvas_->getCurrentlyEditedTextElement();
	if ( textElement ) {
		int elX = textElement->getX();
		int elY = textElement->getY();
		RECT rc  = {elX, elY, elX + textElement->getWidth(), elY + textElement->getHeight()};
		if ( textElement && PtInRect(&rc, pt) ) {
			InputBoxControl* inputBoxControl = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
			inputBoxControl->SendMessage(uMsg, MAKEWPARAM(cx - elX, cy - elY), lParam);
		}
		return 0;
	}*/

	SetCapture();
	//horizontalToolbar_.ShowWindow(SW_HIDE);
	canvas_->mouseDown( 0, cx, cy );
	return 0;
}

LRESULT CImageEditorView::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx =   GET_X_LPARAM(lParam); 
	int cy =   GET_Y_LPARAM(lParam);
	POINT ptScroll;
	GetScrollOffset(ptScroll);
	cx += ptScroll.x;
	cy += ptScroll.y;
	POINT pt = {cx, cy};
	/*TextElement *textElement = canvas_->getCurrentlyEditedTextElement();
	if ( textElement ) {
		int elX = textElement->getX();
		int elY = textElement->getY();
		RECT rc  = {elX, elY, elX + textElement->getWidth(), elY + textElement->getHeight()};
		if ( textElement && PtInRect(&rc, pt) ) {
			InputBoxControl* inputBoxControl = dynamic_cast<InputBoxControl*>(textElement->getInputBox());
			inputBoxControl->SendMessage(uMsg, MAKEWPARAM(cx - elX, cy - elY), lParam);
		}
		return 0;
	}*/
	canvas_->mouseUp( 0, cx, cy );
	ReleaseCapture();
//	horizontalToolbar_.ShowWindow(SW_SHOW);
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
	canvas_->mouseUp( 1, cx, cy );
	ReleaseCapture();
	return 0;
}

LRESULT CImageEditorView::OnLButtonDblClick(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	POINT ptScroll;
	GetScrollOffset(ptScroll);
	cx += ptScroll.x;
	cy += ptScroll.y;
	canvas_->mouseDoubleClick( 0, cx, cy );
	return 0;
}

void CImageEditorView::updateView( Canvas* canvas, const CRgn& region ) {
	POINT pt;
	//CRgn rgn = 
	CRgn rgn = region;

	GetScrollOffset(pt);
	//LOG(INFO) << "ScrollOffset " << pt.x << " " << pt.y;
	rgn.OffsetRgn(-pt.x, -pt.y);
	InvalidateRgn( rgn );
}

LRESULT CImageEditorView::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled) {
	bHandled = true;
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
	
	/*CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_UNDO, TR("Отменить"));
	menu.AppendMenu(MF_STRING, ID_PEN, TR("Карандаш"));
	menu.AppendMenu(MF_STRING, ID_BRUSH, TR("Кисть"));
	menu.AppendMenu(MF_STRING, ID_LINE, TR("Линия"));
	menu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("Прямоугольник"));
	menu.AppendMenu(MF_STRING, ID_TEXT, TR("Добавить текст"));
	menu.AppendMenu(MF_STRING, ID_CROP, TR("Обрезка"));

	//menu.SetMenuDefaultItem(0, true);
	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);*/

	return 0;
}


LRESULT CImageEditorView::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	RECT clientRect;
	GetClientRect(&clientRect);
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	if ( WindowFromPoint(cursorPos ) != m_hWnd ) {
		return 0;
	}
	ScreenToClient(&cursorPos);
	if ( PtInRect(&clientRect, cursorPos ) ) {
		SetCursor(getCachedCursor(canvas_->getCursor()));
	} else {
		SetCursor(getCachedCursor(ctDefault));
	}
	return 0;
}


LRESULT CImageEditorView::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ( wParam == 'Z' && ( !!( ::GetKeyState(VK_LCONTROL) & 0x8000 ) ||  ::GetKeyState(VK_RCONTROL) & 0x8000 ) ) {
		canvas_->undo();
	} if ( wParam == 'D' && ( !!( ::GetKeyState(VK_LCONTROL) & 0x8000 ) ||  ::GetKeyState(VK_RCONTROL) & 0x8000 ) ) {
		canvas_->unselectAllElements();
		canvas_->updateView();
	}else if ( wParam == VK_DELETE ) {
		canvas_->deleteSelectedElements();
	}
	return 0;
}

HCURSOR CImageEditorView::getCachedCursor(CursorType cursorType)
{
	HCURSOR cur = cursorCache_[cursorType];
	if ( cur ) {
		return cur;
	}
	LPCTSTR lpCursorName = 0;
	switch( cursorType ) {
		case ctEdit:
			lpCursorName = IDC_IBEAM;
			break;
		case ctResizeVertical:
			lpCursorName = IDC_SIZENS;
			break;
		case ctResizeHorizontal:
			lpCursorName = IDC_SIZEWE;
			break;
		case ctResizeDiagonalMain:
			lpCursorName = IDC_SIZENWSE;
			break;
		case ctResizeDiagonalAnti:
			lpCursorName = IDC_SIZENESW;
			break;
		case ctCross:
			lpCursorName = IDC_CROSS;
			break;
		case ctMove:
			lpCursorName = IDC_SIZEALL;
			break;
		case ctColorPicker:
			cur = LoadCursor(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDC_COLORPICKERCURSOR));
			cursorCache_[cursorType] = cur;
			return cur;
		default:
			lpCursorName = IDC_ARROW;
	}
	cur = LoadCursor(0, lpCursorName);
	cursorCache_[cursorType] = cur;
	return cur;
}

}
