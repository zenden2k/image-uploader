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
#include <Func/WinUtils.h>
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
	sourceFileName_ = imageFileName;
	init();
}

void ImageEditorWindow::init()
{
//	resultingBitmap_ = 0;
	canvas_ = 0;
	cropToolTip_ = 0;
	showUploadButton_ = true;
	prevPenSize_ = 0;
	imageQuality_ = 85;
	initialDrawingTool_ = Canvas::dtMove;
	menuItems_[ID_PEN]             = Canvas::dtPen; 
	menuItems_[ID_LINE]            = Canvas::dtLine;
	menuItems_[ID_BRUSH]           = Canvas::dtBrush;
	menuItems_[ID_MARKER]          = Canvas::dtMarker;
	menuItems_[ID_RECTANGLE]       = Canvas::dtRectangle;
	menuItems_[ID_FILLEDRECTANGLE] = Canvas::dtFilledRectangle;
	menuItems_[ID_CROP]            = Canvas::dtCrop;
	menuItems_[ID_MOVE]            = Canvas::dtMove;
	menuItems_[ID_ARROW]           = Canvas::dtArrow;
	menuItems_[ID_SELECTION]       = Canvas::dtSelection;
	menuItems_[ID_BLUR]            = Canvas::dtBlur;
	menuItems_[ID_BLURRINGRECTANGLE]   = Canvas::dtBlurrringRectangle;
	menuItems_[ID_COLORPICKER]     = Canvas::dtColorPicker;
	menuItems_[ID_ROUNDEDRECTANGLE]     = Canvas::dtRoundedRectangle;
	menuItems_[ID_ELLIPSE]     = Canvas::dtEllipse;
	menuItems_[ID_FILLEDROUNDEDRECTANGLE]     = Canvas::dtFilledRoundedRectangle;
	menuItems_[ID_FILLEDELLIPSE]     = Canvas::dtFilledEllipse;
	/*		rectangleMenu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("Прямоугольник"));
	rectangleMenu.AppendMenu(MF_STRING, ID_ROUNDEDRECTANGLE, TR("Скругленный прямоугольник"));
	rectangleMenu.AppendMenu(MF_STRING, ID_ELLIPSE, TR("Эллипс"));
	TPMPARAMS excludeArea;
	ZeroMemory(&excludeArea, sizeof(excludeArea));
	excludeArea.cbSize = sizeof(excludeArea);
	excludeArea.rcExclude = rc;
	rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd,&excludeArea);
	} else if ( item->command == ID_FILLEDRECTANGLE || ID_FILLEDROUNDEDRECTANGLE || ID_FILLEDELLIPSE) {
	CMenu rectangleMenu;
	RECT rc = item->rect;
	verticalToolbar_.ClientToScreen(&rc);
	rectangleMenu.CreatePopupMenu();
	rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDRECTANGLE, TR("Заполненный прямоугольник"));
	rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDROUNDEDRECTANGLE, TR("Скругленный прямоугольник"));
	rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDELLIPSE, TR("Эллипс"));
	*/
	SubMenuItem item;
	item.parentCommand = ID_RECTANGLE;
	item.icon = loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG);
	item.command = ID_RECTANGLE;
	item.hint = TR("Прямоугольник");
	subMenuItems_[Canvas::dtRectangle] = item;

	item.icon = loadToolbarIcon(IDB_ICONTOOLROUNDEDRECTANGLE);
	item.command = ID_ROUNDEDRECTANGLE;
	item.hint = TR("Скругленный прямоугольник");
	subMenuItems_[Canvas::dtRoundedRectangle] = item;

	item.icon = loadToolbarIcon(IDB_ICONTOOLELLIPSE);
	item.command = ID_ELLIPSE;
	item.hint = TR("Эллипс");
	subMenuItems_[Canvas::dtEllipse] = item;

	item.parentCommand = ID_FILLEDRECTANGLE;
	item.command = ID_FILLEDRECTANGLE;
	item.hint = TR("Заполненный прямоугольник");
	item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE);
	subMenuItems_[Canvas::dtFilledRectangle] = item;


	item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDROUNDEDRECTANGLE);
	item.command = ID_FILLEDROUNDEDRECTANGLE;
	item.hint = TR("Скругленный прямоугольник");
	subMenuItems_[Canvas::dtFilledRoundedRectangle] = item;


	item.icon = loadToolbarIcon(IDB_ICONTOOLFILLEDELLIPSE);
	item.command = ID_FILLEDELLIPSE;
	item.hint =  TR("Заполненный эллипс");
	subMenuItems_[Canvas::dtFilledEllipse] = item;
	selectedSubMenuItems_[ID_RECTANGLE] = ID_RECTANGLE;
	selectedSubMenuItems_[ID_FILLEDRECTANGLE] = ID_FILLEDRECTANGLE;
	
	dialogResult_ = drCancel;
}

bool ImageEditorWindow::saveDocument()
{
	resultingBitmap_ = canvas_->getBitmapForExport();
	if ( !resultingBitmap_ ) {
		LOG(ERROR) << "canvas_->getBitmapForExport() returned NULL";
		return false;
	}
	if ( !outFileName_.IsEmpty() ) {
		SaveImage(&*resultingBitmap_, outFileName_, sifDetectByExtension, imageQuality_);
	}
	return true;
}

void ImageEditorWindow::updateToolbarDrawingTool(Canvas::DrawingToolType dt)
{
	std::map<Canvas::DrawingToolType, SubMenuItem>::iterator submenuIter = subMenuItems_.find(dt);
	if ( submenuIter != subMenuItems_.end() ) {
		int buttonIndex = verticalToolbar_.getItemIndexByCommand(selectedSubMenuItems_[submenuIter->second.parentCommand]);
		if ( buttonIndex != -1 ) {
			Toolbar::Item* item = verticalToolbar_.getItem(buttonIndex);
			if ( item ) {
				item->icon = submenuIter->second.icon;
				item->command = submenuIter->second.command;
				item->hint = submenuIter->second.hint;
				verticalToolbar_.CreateToolTipForItem(buttonIndex);
				selectedSubMenuItems_[submenuIter->second.parentCommand] = item->command;
			}
			
			verticalToolbar_.clickButton(buttonIndex);
			return;
		}
	}

	//std::map<int, Canvas::DrawingToolType>::iterator it = std::find(menuItems_.begin(), menuItems_.end(), dt);
	std::map<int, Canvas::DrawingToolType>::iterator it;
	for( it = menuItems_.begin(); it != menuItems_.end(); ++it ) {
		if ( it->second == dt ) {
			int buttonIndex = verticalToolbar_.getItemIndexByCommand(it->first);
			if ( buttonIndex != -1 ) {
				verticalToolbar_.clickButton(buttonIndex);
				break;
			}
		}
	}
	if ( dt == Canvas::dtCrop ) {
		createTooltip();
	} else if ( cropToolTip_ ) {
		::DestroyWindow(cropToolTip_);
		cropToolTip_ = 0;
	}
}

void ImageEditorWindow::OnForegroundColorChanged(Gdiplus::Color color)
{
	colorsDelegate_->setForegroundColor(color);
	verticalToolbar_.repaintItem(colorsDelegate_->itemIndex());
}

void ImageEditorWindow::OnBackgroundColorChanged(Gdiplus::Color color)
{
	colorsDelegate_->setBackgroundColor(color);
	verticalToolbar_.repaintItem(colorsDelegate_->itemIndex());
}

ImageEditorWindow::~ImageEditorWindow()
{
	delete canvas_;
	delete currentDoc_;
}

void ImageEditorWindow::setInitialDrawingTool(Canvas::DrawingToolType dt)
{
	initialDrawingTool_ = dt;
}

void ImageEditorWindow::showUploadButton(bool show)
{
	showUploadButton_ = show;
}

ZThread::CountedPtr<Gdiplus::Bitmap> ImageEditorWindow::getResultingBitmap()
{
	return resultingBitmap_;
}

ImageEditorWindow::DialogResult ImageEditorWindow::DoModal(HWND parent, WindowDisplayMode mode)
{

	displayMode_ = mode;
	CRect rc(100,100,1280,800);

	if ( displayMode_ == wdmFullscreen ) {
		GuiTools::GetScreenBounds(rc);
	}

	DWORD windowStyle =  displayMode_ == wdmFullscreen  ?WS_POPUP|WS_CLIPCHILDREN : WS_OVERLAPPED | WS_POPUP | WS_CAPTION |  WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | 
		WS_MINIMIZEBOX|WS_CLIPCHILDREN;
	
	if ( Create(0, rc, _T("Image Editor"), windowStyle, displayMode_ == wdmFullscreen ? WS_EX_TOPMOST : 0) == NULL ) {
		LOG(ERROR) << "Main window creation failed!\n";
		return drCancel;
	}
	//displayMode_ = wdmWindowed;
	ShowWindow(SW_SHOW);
	SetForegroundWindow(m_hWnd);
	//BringWindowToTop();
	//m_view.Invalidate(false);
	
	if ( parent ) {
		::EnableWindow(parent, false);
	}

	CMessageLoop *loop = _Module.GetMessageLoop();
	loop->Run();
	DestroyWindow();
	if ( dialogResult_ == drCancel  ) {
//		delete resultingBitmap_;
		//resultingBitmap_ = 0;
	}
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
	m_view.setCanvas( canvas_ );
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
	canvas_->onBackgroundColorChanged.bind(this, &ImageEditorWindow::OnBackgroundColorChanged);
	canvas_->onCropChanged.bind(this, &ImageEditorWindow::OnCropChanged);
	if ( displayMode_ != wdmWindowed ) {	
		canvas_->onCropFinished.bind(this, &ImageEditorWindow::OnCropFinished);
	}
	
	if ( initialDrawingTool_ != Canvas::dtCrop ) {
		verticalToolbar_.ShowWindow(SW_SHOW);
		horizontalToolbar_.ShowWindow(SW_SHOW);
	}
	canvas_->updateView();
	updatePixelLabel();
	//m_view.Invalidate(false);
	
	canvas_->setDrawingToolType(initialDrawingTool_);
	updateToolbarDrawingTool(initialDrawingTool_);
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
	
	rc.right -=  (displayMode_ == wdmWindowed ? vertToolbarRect.right+kCanvasMargin : 0);
	rc.bottom -=  (displayMode_ == wdmWindowed ? horToolbarRect.bottom+kCanvasMargin : 0);

	m_view.SetWindowPos(0, 0,0, rc.right, rc.bottom, SWP_NOMOVE);
	return 0;
}

LRESULT ImageEditorWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	CPaintDC dc(m_hWnd);
	if ( displayMode_ == wdmWindowed ) {
		
		CRect clientRect;
		GetClientRect(&clientRect);
		CRgn rgn;
		rgn.CreateRectRgn(clientRect.left,clientRect.top, clientRect.right, clientRect.bottom);

		RECT horToolbarRect;
		RECT vertToolbarRect;
		RECT viewRect;
		horizontalToolbar_.GetClientRect(&horToolbarRect);
		horizontalToolbar_.ClientToScreen(&horToolbarRect);
		ScreenToClient(&horToolbarRect);
		verticalToolbar_.GetClientRect(&vertToolbarRect);
		verticalToolbar_.ClientToScreen(&vertToolbarRect);
		ScreenToClient(&vertToolbarRect);
		m_view.GetClientRect(&viewRect);
		m_view.ClientToScreen(&viewRect);
		ScreenToClient(&viewRect);
		
		CRgn horToolRgn;
		CRgn vertToolRgn;
		CRgn viewRgn;
		horToolRgn.CreateRectRgn(horToolbarRect.left, horToolbarRect.top, horToolbarRect.right, horToolbarRect.bottom);
		vertToolRgn.CreateRectRgn(vertToolbarRect.left, vertToolbarRect.top, vertToolbarRect.right, vertToolbarRect.bottom);
		viewRgn.CreateRectRgn(viewRect.left, viewRect.top, viewRect.right, viewRect.bottom);
		rgn.CombineRgn(horToolRgn, RGN_DIFF);
		rgn.CombineRgn(vertToolRgn, RGN_DIFF);
		rgn.CombineRgn(viewRgn, RGN_DIFF);

		CBrush br;
		br.CreateSolidBrush(RGB(255,255,255));
		dc.FillRgn(rgn, br);

		/*CRect viewRect;
		m_view.GetClientRect(&viewRect);*/
		
		/*RECT topRect = {0, 0, clientRect.right,horToolbarRect.bottom + kCanvasMargin};
		dc.FillRect(&topRect, br);
		RECT leftRect = {0, topRect.bottom, vertToolbarRect.right + kCanvasMargin, clientRect.bottom};
		dc.FillRect(&leftRect, br);*/
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
	 if ( wParam == VK_ESCAPE ) {
		EndDialog(drCancel);
		return 0;
	}
	m_view.SendMessage(uMsg, wParam, lParam);
	return 0;
}

LRESULT ImageEditorWindow::OnDropDownClicked(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{	
	Toolbar::Item* item = (Toolbar::Item*)wParam;
	if ( item->command == ID_RECTANGLE || item->command == ID_ROUNDEDRECTANGLE || item->command == ID_ELLIPSE) {
		CMenu rectangleMenu;
		RECT rc = item->rect;
		verticalToolbar_.ClientToScreen(&rc);
		rectangleMenu.CreatePopupMenu();
		rectangleMenu.AppendMenu(MF_STRING, ID_RECTANGLE, TR("Прямоугольник"));
		rectangleMenu.AppendMenu(MF_STRING, ID_ROUNDEDRECTANGLE, TR("Скругленный прямоугольник"));
		rectangleMenu.AppendMenu(MF_STRING, ID_ELLIPSE, TR("Эллипс"));
		TPMPARAMS excludeArea;
		ZeroMemory(&excludeArea, sizeof(excludeArea));
		excludeArea.cbSize = sizeof(excludeArea);
		excludeArea.rcExclude = rc;
		rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd,&excludeArea);
	} else if ( item->command == ID_FILLEDRECTANGLE || ID_FILLEDROUNDEDRECTANGLE || ID_FILLEDELLIPSE) {
		CMenu rectangleMenu;
		RECT rc = item->rect;
		verticalToolbar_.ClientToScreen(&rc);
		rectangleMenu.CreatePopupMenu();
		rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDRECTANGLE, TR("Заполненный прямоугольник"));
		rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDROUNDEDRECTANGLE, TR("Скругленный прямоугольник"));
		rectangleMenu.AppendMenu(MF_STRING, ID_FILLEDELLIPSE, TR("Эллипс"));
		TPMPARAMS excludeArea;
		ZeroMemory(&excludeArea, sizeof(excludeArea));
		excludeArea.cbSize = sizeof(excludeArea);
		excludeArea.rcExclude = rc;
		rectangleMenu.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd,&excludeArea);
	}
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
	RECT rc = {0,0,30,30};
	//GetClientRect(&rc);
	const int IDC_DUMMY = 100;

	if ( !horizontalToolbar_.Create(m_hWnd, displayMode_ == wdmWindowed) ) {
		LOG(ERROR) << "Failed to create horizontal toolbar";
		return;
	}
	horizontalToolbar_.addButton(Toolbar::Item(TR("Добавить в список"),0,ID_ADDTOWIZARD));
	if ( showUploadButton_ ) {
		horizontalToolbar_.addButton(Toolbar::Item(TR("Загрузить на сервер"),0,ID_UPLOAD, CString(), Toolbar::itButton));
	}
	//horizontalToolbar_.addButton(Toolbar::Item(TR("Поделиться"),0,ID_SHARE, CString(),Toolbar::itComboButton));
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
	//verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLSELECTION),ID_SELECTION,TR("Выделение"), Toolbar::itButton, true, 1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_TOOLCROPPING), ID_CROP,TR("Обрезка"), Toolbar::itButton, true,1));
	//verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLPENCIL), ID_PEN,T_R("Карандаш"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_BRUSH,TR("Кисть"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBRUSHPNG), ID_MARKER,TR("Подсвечивающий маркер"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLLINE), ID_LINE,TR("Линия"), Toolbar::itButton, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLARROWPNG), ID_ARROW,TR("Стрелка"), Toolbar::itButton, true,1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLRECTANGLEPNG), ID_RECTANGLE,TR("Прямоугольник"), Toolbar::itTinyCombo, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLFILLEDRECTANGLE), ID_FILLEDRECTANGLE,TR("Заполненный прямоугольник"), Toolbar::itTinyCombo, true,1));
	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLTEXTPNG), ID_TEXT,TR("Текст"), Toolbar::itButton, true,1));

	verticalToolbar_.addButton(Toolbar::Item(CString(),  loadToolbarIcon(IDB_ICONTOOLBLURINGRECTANGLEPNG), ID_BLURRINGRECTANGLE,TR("Размывающий прямоугольник"), Toolbar::itButton, true,1));

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
	horizontalToolbar_.penSizeSlider_.SetPos(canvas_->getPenSize());
}  


void ImageEditorWindow::OnCropChanged(int x, int y, int w, int h)
{
	if ( cropToolTip_ ) {
		::DestroyWindow(cropToolTip_);
		cropToolTip_ = 0;
	}
	if ( displayMode_ != wdmFullscreen ) {
		return;
	}
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
	TCHAR Buf[MAX_PATH*4];
	CString fileExt = WinUtils::GetFileExt(sourceFileName_);
	GuiTools::SelectDialogFilter(Buf, sizeof(Buf)/sizeof(TCHAR),2,
		TR("Файлы")+CString(" *.")+fileExt, CString(_T("*."))+fileExt,
		TR("Все файлы"),_T("*.*"));
	CFileDialog fd(false, fileExt, sourceFileName_,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,Buf,m_hWnd);
	if(fd.DoModal()!=IDOK || !fd.m_szFileName[0]) return 0;

	outFileName_ = fd.m_szFileName;
	saveDocument();
	return 0;
}



bool ImageEditorWindow::createTooltip() {
	// Create a tooltip.
	cropToolTip_ = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, 
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
		m_view.m_hWnd, NULL, _Module.GetModuleInstance(),NULL);

	::SetWindowPos(cropToolTip_, HWND_TOPMOST, 0, 0, 0, 0, 
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Set up "tool" information. In this case, the "tool" is the entire parent window.


	TOOLINFO ti = { 0 };
	ti.cbSize   = sizeof(TOOLINFO);
	ti.uFlags   = TTF_SUBCLASS;
	ti.hwnd     = m_view.m_hWnd;
	ti.hinst    = _Module.GetModuleInstance();
	ti.lpszText =  TR("Выберите область");;
	m_view.GetClientRect(&ti.rect);

	// Associate the tooltip with the "tool" window.
	SendMessage(cropToolTip_, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	return true;
}

void ImageEditorWindow::updatePixelLabel()
{
	horizontalToolbar_.pixelLabel_.SetWindowText(WinUtils::IntToStr(canvas_->getPenSize()) + L" px");
}

LRESULT ImageEditorWindow::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if ( (HWND)lParam != horizontalToolbar_.penSizeSlider_.m_hWnd  ) {
		return 0;
	}
	int penSize = HIWORD(wParam);
	switch( LOWORD(wParam ) ) {
		case TB_THUMBPOSITION:
			LOG(INFO) << "TB_THUMBPOSITION";
			canvas_->endPenSizeChanging(penSize);
			updatePixelLabel();
			prevPenSize_ = 0;
			break;
		case TB_THUMBTRACK:
			if ( !prevPenSize_ ) {
				prevPenSize_ = canvas_->getPenSize();
				canvas_->beginPenSizeChanging();
			}
			canvas_->setPenSize(penSize);
			updatePixelLabel();
			LOG(INFO) << "TB_THUMBTRACK";
			break;

	}
	return 0;
}

}

