// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////


#include "ImageEditorWindow.h"

#include "ImageEditor/resource.h"
#include "ImageEditorView.h"
#include <ImageEditor/Gui/Toolbar.h>
#include <Core/Logging.h>
#include <resource.h>
#include <Core/Images/Utils.h>
#include <3rdpart/ColorButton.h>

namespace ImageEditor {

	class ColorsDelegate: public Toolbar::ToolbarItemDelegate {
	public:
		enum {kOffset = 7, kSquareSize = 16, kPadding = 3};

		ColorsDelegate(Toolbar* toolbar, int itemIndex, Canvas* canvas) {
			toolbar_ = toolbar;
			toolbarItemIndex_ = itemIndex;
			canvas_ = canvas;
			RECT rc = {0,0,1,1};
			
			foregroundButton_.Create(toolbar->m_hWnd, rc, 0,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON);
			foregroundColorButton_.SubclassWindow(foregroundButton_.m_hWnd);
			foregroundColorButton_.OnSelChange.bind(this, &ColorsDelegate::OnForegroundButtonSelChanged);
			foregroundColorButton_.SetCustomText(TR("Больше цветов..."));
			backgroundButton_.Create(toolbar->m_hWnd, rc, 0,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON);
			backgroundColorButton_.SubclassWindow(backgroundButton_.m_hWnd);
			backgroundColorButton_.OnSelChange.bind(this, &ColorsDelegate::OnBackgroundButtonSelChanged);
			backgroundColorButton_.SetCustomText(TR("Больше цветов..."));
		};

		virtual SIZE CalcItemSize(Toolbar::Item& item, float dpiScaleX, float dpiScaleY) {
			SIZE res = { (kSquareSize + kOffset + kPadding )* dpiScaleX,(kSquareSize + kOffset+4)* dpiScaleY};
			return res;
		}

		virtual void DrawItem(Toolbar::Item& item, Gdiplus::Graphics* gr, int x, int y, float dpiScaleX, float dpiScaleY) {
			using namespace Gdiplus;
			Pen borderPen(Color(0,0,0));
		
			SolidBrush backgroundBrush(backgroundColor_);
			SolidBrush foregroundBrush(foregroundColor_);

			backgroundRect_ = Rect(x+(kPadding+kOffset)*dpiScaleX, y+kOffset*dpiScaleY, kSquareSize * dpiScaleX, kSquareSize*  dpiScaleY);
			gr->FillRectangle(&backgroundBrush, backgroundRect_);
			gr->DrawRectangle(&borderPen, backgroundRect_);

			foregroundRect_ = Rect(kPadding*dpiScaleX + x, y, kSquareSize * dpiScaleX, kSquareSize *  dpiScaleY);
			gr->FillRectangle(&foregroundBrush, foregroundRect_);
			gr->DrawRectangle(&borderPen, foregroundRect_);

			POINT pt = {foregroundRect_.X,foregroundRect_.Y + foregroundRect_.Height};
			//toolbar_->ClientToScreen(&pt);
			foregroundColorButton_.SetWindowPos(0, pt.x, pt.y,0,0,/*SWP_NOSIZE*/0);

			POINT pt2 = {backgroundRect_.X,backgroundRect_.Y + backgroundRect_.Height};
			//toolbar_->ClientToScreen(&pt2);
			backgroundColorButton_.SetWindowPos(0, pt2.x, pt2.y,0,0,/*SWP_NOSIZE*/0);
		}

		void setForegroundColor(Gdiplus::Color color ) {
			foregroundColor_ = color;
		}

		void setBackgroundColor(Gdiplus::Color color) {
			backgroundColor_ = color;
		}
	
		Gdiplus::Color getForegroundColor() const {
			return foregroundColor_;
		}

		Gdiplus::Color getBackgroundColor() const {
			return backgroundColor_;
		}

		virtual void OnClick(int x, int y, float dpiScaleX, float dpiScaleY){
			if ( foregroundRect_.Contains(x,y) ) {
				//MessageBox(0,0,0,0);
				foregroundColorButton_.Click();
			} else if ( backgroundRect_.Contains(x,y)) {
				backgroundColorButton_.Click();
			}
			
			//MessageBox(0,0,0,0);
		};

		protected:
			Gdiplus::Color foregroundColor_;
			Gdiplus::Color backgroundColor_;
			Toolbar* toolbar_;
			CColorButton foregroundColorButton_;
			CColorButton backgroundColorButton_;
			CButton foregroundButton_;
			CButton backgroundButton_;
			Gdiplus::Rect backgroundRect_;
			Gdiplus::Rect foregroundRect_;
			int toolbarItemIndex_;
			Canvas* canvas_;

			void OnForegroundButtonSelChanged(COLORREF color, BOOL valid ) {
				foregroundColor_ = Gdiplus::Color(GetRValue(color), GetGValue(color), GetBValue(color));
				toolbar_->repaintItem(toolbarItemIndex_);
				canvas_->setForegroundColor(foregroundColor_);
				
			}

			void OnBackgroundButtonSelChanged(COLORREF color, BOOL valid ) {
				backgroundColor_ = Gdiplus::Color(GetRValue(color), GetGValue(color), GetBValue(color));
				toolbar_->repaintItem(toolbarItemIndex_);
				canvas_->setBackgroundColor(backgroundColor_);
			}

	};

ImageEditorWindow::ImageEditorWindow():horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
	canvas_ = 0;
	menuItems_[ID_PEN].toolId       = Canvas::dtPen; 
	menuItems_[ID_LINE].toolId      = Canvas::dtLine;
	menuItems_[ID_BRUSH].toolId     = Canvas::dtBrush;
	menuItems_[ID_RECTANGLE].toolId = Canvas::dtRectangle;
	menuItems_[ID_FILLEDRECTANGLE].toolId = Canvas::dtFilledRectangle;
	menuItems_[ID_CROP].toolId      = Canvas::dtCrop;
	menuItems_[ID_MOVE].toolId      = Canvas::dtMove;
	menuItems_[ID_ARROW].toolId      = Canvas::dtArrow;
	menuItems_[ID_SELECTION].toolId  = Canvas::dtSelection;
	menuItems_[ID_BLUR].toolId      = Canvas::dtBlur;
	menuItems_[ID_BLURRINGRECTANGLE].toolId     = Canvas::dtBlurrringRectangle;

	
}

ImageEditorWindow::~ImageEditorWindow()
{
	delete canvas_;
}

LRESULT ImageEditorWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	RECT rc = {0,0,1210,733};
	GetClientRect(&rc);
	HWND m_hWndClient = m_view.Create(m_hWnd, rc, _T("ImageEditor_Canvas"), WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, 0 );
		
		//Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

	currentDoc_ = new ImageEditor::Document(L"screenshot.png");

	
	//ImageEditor::Line line( 0, 0, 50, 50 );
	//line.resize( Gdiplus::Rect( 0, 0, 50, 50 ) );
	//currentDoc_->addDrawingElement( &line );
	canvas_ = new ImageEditor::Canvas( m_hWnd );
	canvas_->setSize( currentDoc_->getWidth(), currentDoc_->getHeight());
	canvas_->setDocument( currentDoc_ );
	createToolbars();
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

LRESULT ImageEditorWindow::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_view.SendMessage(uMsg, wParam, lParam);
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

	if ( !horizontalToolbar_.Create(m_hWnd) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";
	
	}
	horizontalToolbar_.addButton(Toolbar::Item(TR("Добавить в список"),0,100));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Загрузить на сервер"),0,100, CString(), Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Поделиться"),0,100, CString(),Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Сохранить"),0,100, CString(),Toolbar::itButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Закрыть"),0,100));
	horizontalToolbar_.AutoSize();
	horizontalToolbar_.ShowWindow(SW_SHOW);

	if ( !verticalToolbar_.Create(m_hWnd) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";

	}

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLMOVEICONPNG),ID_MOVE,TR("Перемещение"), Toolbar::itButton, true, 1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLSELECTION),ID_SELECTION,TR("Выделение"), Toolbar::itButton, true, 1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLCROPPING), ID_CROP,TR("Обрезка"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLPENCIL), ID_PEN,TR("Карандаш"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BRUSH,TR("Кисть"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLLINE), ID_LINE,TR("Линия"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLARROWPNG), ID_ARROW,TR("Стрелка"), Toolbar::itButton, true,1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG), ID_RECTANGLE,TR("Прямоугольник"), Toolbar::itTinyCombo, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_FILLEDRECTANGLE,TR("Заполненный прямоугольник"), Toolbar::itTinyCombo, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLTEXTPNG), ID_TEXT,TR("Текст"), Toolbar::itButton, true,1));
#if GDIPVER >= 0x0110 
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_BLURRINGRECTANGLE,TR("Размывающий прямоугольник"), Toolbar::itComboButton, true,1));
#endif
	//verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BLUR,TR("Размытие"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONCOLORPICKERPNG), ID_COLORPICKER,TR("Выбрать цвет"), Toolbar::itButton, true,1));
	int index = verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,TR("Отменить") + CString(L" (Ctrl+Z)"), Toolbar::itButton, false));

	Toolbar::Item colorsButton(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,CString(), Toolbar::itButton, false);
	ColorsDelegate* delegate_ = new ColorsDelegate(&verticalToolbar_, index+1, canvas_);
	colorsButton.itemDelegate = delegate_;
	delegate_->setBackgroundColor(canvas_->getBackgroundColor());
	delegate_->setForegroundColor(canvas_->getForegroundColor());
	verticalToolbar_.addButton(colorsButton);

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