// ImageEditorView.cpp : implementation of the CImageEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#include "ImageEditorView.h"

#include <algorithm>
#include "ImageEditor/BasicElements.h"
#include <GdiPlus.h>
#include <Gui/GuiTools.h>
#include <Core/Logging.h>
#include <Core/Images/Utils.h>
#include "resource.h"

#ifndef TR
#define TR(a) L##a
#endif
namespace ImageEditor {

	CImageEditorView::CImageEditorView() :horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical)  {
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
	//return 0;
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
	//horizontalToolbar_.Invalidate(TRUE);

	return 0;
}


LRESULT CImageEditorView::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	createToolbars();
	return 0;
}

void CImageEditorView::setCanvas(ImageEditor::Canvas *canvas) {
	canvas_ = canvas;
	if ( canvas ) {
		canvas_->setSize( 1280, 720 );
		canvas_->setCallback( this );
		canvas_->setDrawingToolType(Canvas::dtRectangle);
		canvas_->onCropChanged.bind(this, &CImageEditorView::OnCropChanged);
	}
}

LRESULT CImageEditorView::OnMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	POINT pt = {cx, cy};
	RECT toolBarRect;
	horizontalToolbar_.GetClientRect(&toolBarRect);
	horizontalToolbar_.ClientToScreen(&toolBarRect);
	ClientToScreen(&pt);
	/*if ( pt.x >= toolBarRect.left && pt.x <= toolBarRect.right && pt.y >= toolBarRect.top && pt.y <= toolBarRect.bottom ) {
		return 0;
	}*/
//	HWND wnd =  WindowFromPoint(pt);
	/*if ( wnd == m_hWnd )*/ {
		canvas_->mouseMove( cx, cy, wParam );
	}
	return 0;
}

LRESULT CImageEditorView::OnLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	SetCapture();
	//horizontalToolbar_.ShowWindow(SW_HIDE);
	canvas_->mouseDown( 0, cx, cy );
	return 0;
}

LRESULT CImageEditorView::OnLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/) {
	int cx = LOWORD(lParam); 
	int cy = HIWORD(lParam);
	canvas_->mouseUp( 0, cx, cy );
	ReleaseCapture();
	horizontalToolbar_.ShowWindow(SW_SHOW);
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
	menu.AppendMenu(MF_STRING, ID_UNDO, TR("��������"));
	menu.AppendMenu(MF_STRING, ID_PEN, TR("��������"));
	menu.AppendMenu(MF_STRING, ID_BRUSH, TR("�����"));
	menu.AppendMenu(MF_STRING, ID_LINE, TR("�����"));
	menu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("�������������"));
	menu.AppendMenu(MF_STRING, ID_TEXT, TR("�������� �����"));
	menu.AppendMenu(MF_STRING, ID_CROP, TR("�������"));

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
		default:
			lpCursorName = IDC_ARROW;
	}
	cur = LoadCursor(0, lpCursorName);
	cursorCache_[cursorType] = cur;
	return cur;
}

void CImageEditorView::createToolbars()
{
	toolbarImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
	RECT rc = {0,0,500,30};
	//GetClientRect(&rc);
	const int IDC_DUMMY = 100;
	/*rc.top = rc.bottom - GuiTools::dlgY(13);
	rc.bottom-= GuiTools::dlgY(1);
	rc.left = GuiTools::dlgX(3);
	rc.right -= GuiTools::dlgX(3);*/
	if ( !horizontalToolbar_.Create(m_hWnd) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";
	
	}
	horizontalToolbar_.addButton(Toolbar::Item(TR("�������� � ������"),0,100));
	horizontalToolbar_.addButton(Toolbar::Item(TR("��������� �� ������"),0,100, CString(), Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("����������"),0,100, CString(),Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("�������"),0,100));
	horizontalToolbar_.AutoSize();
	horizontalToolbar_.ShowWindow(SW_SHOW);

	if ( !verticalToolbar_.Create(m_hWnd) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";

	}

	/*menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_UNDO, TR("��������"));
	menu.AppendMenu(MF_STRING, ID_PEN, TR("��������"));
	menu.AppendMenu(MF_STRING, ID_BRUSH, TR("�����"));
	menu.AppendMenu(MF_STRING, ID_LINE, TR("�����"));
	menu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("�������������"));
	menu.AppendMenu(MF_STRING, ID_TEXT, TR("�������� �����"));
	menu.AppendMenu(MF_STRING, ID_CROP, TR("�������"));*/

	verticalToolbar_.addButton(Toolbar::Item(CString(),  BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(ID_CROP),_T("PNG")) ,100,TR("�����������")));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDB_TOOLCROPPING),_T("PNG")) ,ID_CROP,TR("�������")));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDB_ICONTOOLPENCIL),_T("PNG")) ,ID_PEN,TR("��������")));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDB_ICONTOOLBRUSHPNG),_T("PNG")) ,ID_BRUSH,TR("�����")));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(IDB_ICONTOOLTEXTPNG),_T("PNG")) ,ID_BRUSH,TR("�����")));

	
	verticalToolbar_.AutoSize();
	verticalToolbar_.ShowWindow(SW_SHOW);
}

void CImageEditorView::OnCropChanged(int x, int y, int w, int h)
{
	enum ToolbarPosition { pBottomRight, pTopLeft, pBottomInner };
	ToolbarPosition pos = pBottomRight ;
	RECT rc, vertRc;
	horizontalToolbar_.GetClientRect(&rc);
	verticalToolbar_.GetClientRect(&vertRc);

	if ( y + h + rc.bottom > canvas_->getHeigth()   ) {
		pos = pTopLeft;
	}
	POINT horToolbarPos = {0,0};
	POINT vertToolbarPos = {0,0};
	if ( pos == pBottomRight ) {
		horToolbarPos.x = x + w - rc.right;
		horToolbarPos.y =  y + h + 6;

		vertToolbarPos.x = x + w + 6 ;
		vertToolbarPos.y = y + h - vertRc.bottom;
	} else if ( pos == pTopLeft ) {
		horToolbarPos.x = x;
		horToolbarPos.y =  y - rc.bottom-6;

		vertToolbarPos.x = x - vertRc.right - 6;
		vertToolbarPos.y = y ;
	}
	ClientToScreen(&horToolbarPos);
	ClientToScreen(&vertToolbarPos);

	horizontalToolbar_.SetWindowPos(0, horToolbarPos.x, horToolbarPos.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	//horizontalToolbar_.SetWindowPos(0, horToolbarPos.x, horToolbarPos.y, 0, 0, SWP_NOSIZE);
	verticalToolbar_.SetWindowPos(0, vertToolbarPos.x, vertToolbarPos.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);

}

}
