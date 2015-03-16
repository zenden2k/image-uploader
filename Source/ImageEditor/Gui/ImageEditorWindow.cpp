// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////


#include "ImageEditorWindow.h"

#include "ImageEditor/resource.h"
#include "ImageEditorView.h"
#include "ImageEditor/BasicElements.h"
#include <ImageEditor/Gui/Toolbar.h>
#include <Core/Logging.h>
#include <resource.h>
#include <Core/Images/Utils.h>
namespace ImageEditor {

ImageEditorWindow::ImageEditorWindow():horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
	canvas_ = 0;
	menuItems_[ID_PEN].toolId       = Canvas::dtPen; 
	menuItems_[ID_LINE].toolId      = Canvas::dtLine;
	menuItems_[ID_BRUSH].toolId     = Canvas::dtBrush;
	menuItems_[ID_RECTANGLE].toolId = Canvas::dtRectangle;
	menuItems_[ID_CROP].toolId      = Canvas::dtCrop;
	menuItems_[ID_MOVE].toolId      = Canvas::dtMove;
}

ImageEditorWindow::~ImageEditorWindow()
{
	delete canvas_;
}

LRESULT ImageEditorWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	RECT rc = {0,0,1024,720};
	GetClientRect(&rc);
	HWND m_hWndClient = m_view.Create(m_hWnd, rc, _T("ImageEditor_Canvas"), WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, 0 );
		
		//Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	currentDoc_ = new ImageEditor::Document(L"screenshot.png");

	createToolbars();
	//ImageEditor::Line line( 0, 0, 50, 50 );
	//line.resize( Gdiplus::Rect( 0, 0, 50, 50 ) );
	//currentDoc_->addDrawingElement( &line );
	canvas_ = new ImageEditor::Canvas( m_hWnd );
	canvas_->setSize( currentDoc_->getWidth(), currentDoc_->getHeight());
	canvas_->setDocument( currentDoc_ );
	canvas_->onCropChanged.bind(this, &ImageEditorWindow::OnCropChanged);

	m_view.setCanvas( canvas_ );
	canvas_->setDrawingToolType(ImageEditor::Canvas::dtCrop);
	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	//pLoop->AddMessageFilter(this);
	//pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT ImageEditorWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	// unregister message filtering and idle updates


	bHandled = FALSE;
	return 1;
}

LRESULT ImageEditorWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	PostQuitMessage(0);
	return 0;
}

LRESULT ImageEditorWindow::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT ImageEditorWindow::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: add code to initialize document

	return 0;
}

LRESULT ImageEditorWindow::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	/*BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
	::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();*/
	return 0;
}

LRESULT ImageEditorWindow::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}


void ImageEditorWindow::createToolbars()
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
	horizontalToolbar_.addButton(Toolbar::Item(TR("Добавить в список"),0,100));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Загрузить на сервер"),0,100, CString(), Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Поделиться"),0,100, CString(),Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Закрыть"),0,100));
	horizontalToolbar_.AutoSize();
	horizontalToolbar_.ShowWindow(SW_SHOW);

	if ( !verticalToolbar_.Create(m_hWnd) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";

	}

	/*menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, ID_UNDO, TR("Отменить"));
	menu.AppendMenu(MF_STRING, ID_PEN, TR("Карандаш"));
	menu.AppendMenu(MF_STRING, ID_BRUSH, TR("Кисть"));
	menu.AppendMenu(MF_STRING, ID_LINE, TR("Линия"));
	menu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("Прямоугольник"));
	menu.AppendMenu(MF_STRING, ID_TEXT, TR("Добавить текст"));
	menu.AppendMenu(MF_STRING, ID_CROP, TR("Обрезка"));*/

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLMOVEICONPNG),ID_MOVE,TR("Перемещение"), Toolbar::itButton, true, 1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLCROPPING), ID_CROP,TR("Обрезка"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLPENCIL), ID_PEN,TR("Карандаш"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BRUSH,TR("Кисть"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLLINE), ID_LINE,TR("Линия"), Toolbar::itButton, true,1));
	//IDB_ICONTOOLRECTANGLEPNG
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG), ID_RECTANGLE,TR("Прямоугольник"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_RECTANGLE,TR("Заполненный прямоугольник"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLTEXTPNG), ID_TEXT,TR("Текст"), Toolbar::itButton, true,1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONCOLORPICKERPNG), ID_COLORPICKER,TR("Выбрать цвет"), Toolbar::itButton, true,1));

	
	LOG(INFO) << "m_hWnd="<<m_hWnd;
	
	verticalToolbar_.AutoSize();
	verticalToolbar_.ShowWindow(SW_SHOW);
}


void ImageEditorWindow::OnCropChanged(int x, int y, int w, int h)
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



Gdiplus::Bitmap * ImageEditorWindow::loadToolbarIcon(int resource)
{
	return BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(resource),_T("PNG")) ;
}

LRESULT ImageEditorWindow::OnMenuItemClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	int toolId = menuItems_[wID].toolId;
	canvas_->setDrawingToolType( static_cast<Canvas::DrawingToolType>( toolId ) );
	return 0;
}

LRESULT ImageEditorWindow::OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	canvas_->undo();
	return 0;
}


LRESULT ImageEditorWindow::OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {

	canvas_->setDrawingToolType( Canvas::dtText );
	return 0;
}

}