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
#include <Gui/GuiTools.h>

namespace ImageEditor {
	class ColorsDelegate: public Toolbar::ToolbarItemDelegate {
	public:
		enum {kOffset = 7, kSquareSize = 16, kPadding = 3};

		ColorsDelegate(Toolbar* toolbar, int itemIndex, Canvas* canvas) {
			toolbar_ = toolbar;
			toolbarItemIndex_ = itemIndex;
			canvas_ = canvas;
			RECT rc = {0,0,1,1};
			font_ = GuiTools::GetSystemDialogFont();
			foregroundButton_.Create(toolbar->m_hWnd, rc, 0,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON);
			foregroundButton_.SetFont(font_);
			foregroundColorButton_.SubclassWindow(foregroundButton_.m_hWnd);
			foregroundColorButton_.OnSelChange.bind(this, &ColorsDelegate::OnForegroundButtonSelChanged);
			foregroundColorButton_.SetCustomText(TR("Больше цветов..."));
			backgroundButton_.Create(toolbar->m_hWnd, rc, 0,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON);
			backgroundButton_.SetFont(font_);
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

		int itemIndex() {
			return toolbarItemIndex_;
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
		CFont font_;

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


ImageEditorWindow::ImageEditorWindow(Gdiplus::Bitmap * bitmap):horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
	currentDoc_ =  new ImageEditor::Document(bitmap);
	init();
	
}

ImageEditorWindow::ImageEditorWindow(CString imageFileName):horizontalToolbar_(Toolbar::orHorizontal),verticalToolbar_(Toolbar::orVertical) 
{
	currentDoc_ = new ImageEditor::Document(imageFileName);
	init();
}

void ImageEditorWindow::init()
{
	canvas_ = 0;
	menuItems_[ID_PEN]             = Canvas::dtPen; 
	menuItems_[ID_LINE]            = Canvas::dtLine;
	menuItems_[ID_BRUSH]           = Canvas::dtBrush;
	menuItems_[ID_RECTANGLE]       = Canvas::dtRectangle;
	menuItems_[ID_FILLEDRECTANGLE] = Canvas::dtFilledRectangle;
	menuItems_[ID_CROP]            = Canvas::dtCrop;
	menuItems_[ID_MOVE]            = Canvas::dtMove;
	menuItems_[ID_ARROW]           = Canvas::dtArrow;
	menuItems_[ID_SELECTION]       = Canvas::dtSelection;
	menuItems_[ID_BLUR]            = Canvas::dtBlur;
	menuItems_[ID_BLURRINGRECTANGLE]   = Canvas::dtBlurrringRectangle;
	menuItems_[ID_COLORPICKER]     = Canvas::dtColorPicker;
	dialogResult_ = drCancel;
}

bool ImageEditorWindow::saveDocument()
{
	return true;
}

void ImageEditorWindow::updateToolbarDrawingTool(Canvas::DrawingToolType dt)
{
	//std::map<int, Canvas::DrawingToolType>::iterator it = std::find(menuItems_.begin(), menuItems_.end(), dt);
	std::map<int, Canvas::DrawingToolType>::iterator it;
	for( it = menuItems_.begin(); it != menuItems_.end(); ++it ) {
		if ( it->second == dt ) {
			int buttonIndex = verticalToolbar_.getItemIndexByCommand(it->first);
			if ( buttonIndex != -1 ) {
				verticalToolbar_.clickButton(buttonIndex);
				return;
			}
		}
	}
}

void ImageEditorWindow::OnForegroundColorChanged(Gdiplus::Color color)
{
	colorsDelegate_->setForegroundColor(color);
	verticalToolbar_.repaintItem(colorsDelegate_->itemIndex());
}

ImageEditorWindow::~ImageEditorWindow()
{
	delete canvas_;
	delete currentDoc_;
}

ImageEditorWindow::DialogResult ImageEditorWindow::DoModal(HWND parent, WindowDisplayMode mode)
{
	mode = wdmWindowed;
//	mode = wdmFullscreen;
	displayMode_ = mode;
	CRect rc(100,100,1280,800);

	if ( displayMode_ == wdmFullscreen ) {
		GuiTools::GetScreenBounds(rc);
	}

	DWORD windowStyle =  displayMode_ == wdmFullscreen ? /*WS_OVERLAPPED|*/ WS_POPUP : WS_OVERLAPPED | WS_POPUP | WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
		WS_MINIMIZEBOX;

	if ( Create(0, rc, _T("Image Editor"), windowStyle, displayMode_ == wdmFullscreen ? WS_EX_TOPMOST : 0) == NULL ) {
		LOG(ERROR) << "Main window creation failed!\n";
		return drCancel;
	}
	ShowWindow(SW_SHOWNORMAL);
	if ( parent ) {
		::EnableWindow(parent, false);
	}

	CMessageLoop *loop = _Module.GetMessageLoop();
	loop->Run();
	DestroyWindow();
	if ( parent ) {
		::EnableWindow(parent, true);
	}
	return dialogResult_;
}

LRESULT ImageEditorWindow::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if ( displayMode_ == wdmWindowed ) {
		CenterWindow();
	}
	int iconWidth =  ::GetSystemMetrics(SM_CXICON);
	if ( iconWidth > 32 ) {
		iconWidth = 48;
	}
	icon_ = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, iconWidth, iconWidth, LR_DEFAULTCOLOR);
	SetIcon(icon_, TRUE);
	int iconSmWidth =  ::GetSystemMetrics(SM_CXSMICON);
	if ( iconSmWidth > 16 ) {
		iconSmWidth = 32;
	}
	iconSmall_ = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, iconSmWidth, iconSmWidth, LR_DEFAULTCOLOR);
	SetIcon(iconSmall_, FALSE);

	RECT rc;
	GetClientRect(&rc);
	HWND m_hWndClient = m_view.Create(m_hWnd, rc, _T("ImageEditor_Canvas"), WS_CHILD | WS_VISIBLE /*| WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/, 0 );

	canvas_ = new ImageEditor::Canvas( m_view );
	canvas_->setSize( currentDoc_->getWidth(), currentDoc_->getHeight());
	canvas_->setDocument( currentDoc_ );

	createToolbars();
	RECT horToolbarRect;
	RECT vertToolbarRect;
	horizontalToolbar_.GetClientRect(&horToolbarRect);
	verticalToolbar_.GetClientRect(&vertToolbarRect);
	rc.left =  displayMode_ == wdmWindowed ? vertToolbarRect.right + kCanvasMargin : 0;
	rc.top =  displayMode_ == wdmWindowed ? horToolbarRect.bottom + kCanvasMargin  : 0;
	if ( displayMode_ == wdmWindowed ) {
		horizontalToolbar_.SetWindowPos(0, rc.left, 0,0,0, SWP_NOSIZE);
		verticalToolbar_.SetWindowPos(0, 0, rc.top, 0,0, SWP_NOSIZE);
	}
	m_view.SetWindowPos(0, &rc, SWP_NOSIZE);
	//,1210,733};
	
	//Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);



	//ImageEditor::Line line( 0, 0, 50, 50 );
	//line.resize( Gdiplus::Rect( 0, 0, 50, 50 ) );
	//currentDoc_->addDrawingElement( &line );
	
	canvas_->onDrawingToolChanged.bind(this, &ImageEditorWindow::OnDrawingToolChanged);
	canvas_->onForegroundColorChanged.bind(this, &ImageEditorWindow::OnForegroundColorChanged);
	if ( displayMode_ != wdmWindowed ) {
		canvas_->onCropChanged.bind(this, &ImageEditorWindow::OnCropChanged);
		canvas_->onCropFinished.bind(this, &ImageEditorWindow::OnCropFinished);
	}

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
	EndDialog(drCancel);
	//PostQuitMessage(0);
	return 0;
}

LRESULT ImageEditorWindow::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	RECT horToolbarRect;
	RECT vertToolbarRect;
	horizontalToolbar_.GetClientRect(&horToolbarRect);
	verticalToolbar_.GetClientRect(&vertToolbarRect);
	rc.right -=  displayMode_ == wdmWindowed ? vertToolbarRect.right+kCanvasMargin : 0;
	rc.bottom -=  displayMode_ == wdmWindowed ? horToolbarRect.bottom+kCanvasMargin : 0;

	m_view.SetWindowPos(0, 0,0, rc.right, rc.bottom, SWP_NOMOVE);
	return 0;
}

LRESULT ImageEditorWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	if ( displayMode_ == wdmWindowed ) {
		CPaintDC dc(m_hWnd);
		CRect clientRect;
		GetClientRect(&clientRect);
		RECT horToolbarRect;
		RECT vertToolbarRect;
		horizontalToolbar_.GetClientRect(&horToolbarRect);
		verticalToolbar_.GetClientRect(&vertToolbarRect);

		/*CRect viewRect;
		m_view.GetClientRect(&viewRect);*/
		CBrush br;
		br.CreateSolidBrush(RGB(255,255,255));
		RECT topRect = {0, 0, clientRect.right,horToolbarRect.bottom + kCanvasMargin};
		dc.FillRect(&topRect, br);
		RECT leftRect = {0, topRect.bottom, vertToolbarRect.right + kCanvasMargin, clientRect.bottom};
		dc.FillRect(&leftRect, br);
	}

	return 0;
}

LRESULT ImageEditorWindow::OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true;
	return TRUE;
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

	if ( !horizontalToolbar_.Create(m_hWnd, displayMode_ == wdmWindowed) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";
		return;
	}
	horizontalToolbar_.addButton(Toolbar::Item(TR("Добавить в список"),0,ID_ADDTOWIZARD));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Загрузить на сервер"),0,ID_UPLOAD, CString(), Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Поделиться"),0,ID_SHARE, CString(),Toolbar::itComboButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Сохранить"),0,ID_SAVE, CString(),Toolbar::itButton));
	horizontalToolbar_.addButton(Toolbar::Item(TR("Закрыть"),0,ID_CLOSE));
	horizontalToolbar_.AutoSize();
	if ( displayMode_ != wdmFullscreen ) {
		horizontalToolbar_.ShowWindow(SW_SHOW);
	}

	if ( !verticalToolbar_.Create(m_hWnd, displayMode_ == wdmWindowed) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";

	}
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLMOVEICONPNG),ID_MOVE,TR("Перемещение"), Toolbar::itButton, true, 1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLSELECTION),ID_SELECTION,TR("Выделение"), Toolbar::itButton, true, 1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLCROPPING), ID_CROP,TR("Обрезка"), Toolbar::itButton, true,1));
	//verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLPENCIL), ID_PEN,T_R("Карандаш"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BRUSH,TR("Кисть"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLLINE), ID_LINE,TR("Линия"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLARROWPNG), ID_ARROW,TR("Стрелка"), Toolbar::itButton, true,1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG), ID_RECTANGLE,TR("Прямоугольник"), Toolbar::itTinyCombo, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_FILLEDRECTANGLE,TR("Заполненный прямоугольник"), Toolbar::itTinyCombo, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLTEXTPNG), ID_TEXT,TR("Текст"), Toolbar::itButton, true,1));
#if GDIPVER >= 0x0110 
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBLURINGRECTANGLEPNG), ID_BLURRINGRECTANGLE,TR("Размывающий прямоугольник"), Toolbar::itButton, true,1));
#endif
	//verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BLUR,TR("Размытие"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONCOLORPICKERPNG), ID_COLORPICKER,TR("Выбрать цвет"), Toolbar::itButton, true,1));
	int index = verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,TR("Отменить") + CString(L" (Ctrl+Z)"), Toolbar::itButton, false));

	Toolbar::Item colorsButton(CString(),  loadToolbarIcon(IDB_ICONUNDOPNG), ID_UNDO,CString(), Toolbar::itButton, false);
	colorsDelegate_ = new ColorsDelegate(&verticalToolbar_, index+1, canvas_);
	colorsButton.itemDelegate = colorsDelegate_;
	colorsDelegate_->setBackgroundColor(canvas_->getBackgroundColor());
	colorsDelegate_->setForegroundColor(canvas_->getForegroundColor());
	verticalToolbar_.addButton(colorsButton);

	verticalToolbar_.AutoSize();
	if ( displayMode_ != wdmFullscreen ) {
		verticalToolbar_.ShowWindow(SW_SHOW);
	}
}


void ImageEditorWindow::OnCropChanged(int x, int y, int w, int h)
{
	enum ToolbarPosition { pBottomRight, pTopLeft, pBottomInner };
	ToolbarPosition pos = pBottomRight ;
	POINT scrollOffset;
	m_view.GetScrollOffset(scrollOffset);
	x -= scrollOffset.x;
	y -= scrollOffset.y;
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



void ImageEditorWindow::OnCropFinished(int x, int y, int w, int h)
{
	OnCropChanged(x,y,w,h);
	if ( !verticalToolbar_.IsWindowVisible() ) {
		verticalToolbar_.ShowWindow(SW_SHOW);
		horizontalToolbar_.ShowWindow(SW_SHOW);
	}
}

void ImageEditorWindow::OnDrawingToolChanged(Canvas::DrawingToolType drawingTool)
{
	updateToolbarDrawingTool(drawingTool);
	SendMessage(WM_SETCURSOR,0,0);
}

Gdiplus::Bitmap * ImageEditorWindow::loadToolbarIcon(int resource)
{
	return BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(resource),_T("PNG")) ;
}

void ImageEditorWindow::EndDialog(DialogResult dr)
{
	dialogResult_ = dr;
	PostQuitMessage(0);
}



LRESULT ImageEditorWindow::OnMenuItemClick(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	Canvas::DrawingToolType toolId =  static_cast<Canvas::DrawingToolType>(menuItems_[wID]);
	canvas_->setDrawingToolType( toolId  );
	updateToolbarDrawingTool(toolId);
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

LRESULT ImageEditorWindow::OnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(drCancel);
	return 0;
}

LRESULT ImageEditorWindow::OnClickedAddToWizard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( saveDocument() ) {
		EndDialog(drAddToWizard);
	}
	return 0;
}

LRESULT ImageEditorWindow::OnClickedUpload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( saveDocument() ) {
		EndDialog(drUpload);
	}
	return 0;
}

LRESULT ImageEditorWindow::OnClickedShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if ( saveDocument() ) {
		EndDialog(drShare);
	}
	return 0;
}

LRESULT ImageEditorWindow::OnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}

}

