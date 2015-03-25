#ifndef IMAGEEDITOR_MAINFRM_H
#define IMAGEEDITOR_MAINFRM_H

#include "ImageEditor/resource.h"
#include "ImageEditorView.h"
#include "ImageEditor/Document.h"
#include "Toolbar.h"
#include "TextParamsWindow.h"
#include <Core/Utils/CoreTypes.h>
#include <GdiPlus.h>
// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
namespace ImageEditor {

class ColorsDelegate;

class ConfigurationProvider;

class ImageEditorWindow : public CWindowImpl<ImageEditorWindow>
{
public:
	DECLARE_WND_CLASS(_T("ImageEditorWindow"))
	enum { ID_UNDO = 1000,  ID_CLOSE, ID_ADDTOWIZARD, ID_UPLOAD, ID_SHARE , ID_SAVE, ID_SAVEAS,
		ID_PEN = 1600, 
		ID_BRUSH, ID_MARKER,ID_BLUR, ID_BLURRINGRECTANGLE, ID_LINE, ID_ARROW, ID_RECTANGLE,  ID_ROUNDEDRECTANGLE, ID_ELLIPSE,
		ID_FILLEDRECTANGLE, ID_FILLEDROUNDEDRECTANGLE, ID_FILLEDELLIPSE, ID_COLORPICKER, ID_CROP , ID_SELECTION,ID_TEXT, ID_MOVE};
	struct MenuItem {
		int menuItemId;
		int toolId;
	};

	struct SubMenuItem {
		int command;
		int parentCommand;
		Gdiplus::Bitmap* icon;
		CString hint;
	};

	enum { kCanvasMargin = 4 }; // margin between toolbars and canvas in windowed mode

	enum DialogResult{
		drCancel, drAddToWizard, drUpload, drShare, drSave
	};

	enum WindowDisplayMode {
		wdmAuto, wdmFullscreen, wdmWindowed
	};

	ImageEditor::CImageEditorView m_view;

	ImageEditorWindow(Gdiplus::Bitmap * bitmap, bool hasTransparentPixels, ConfigurationProvider* configurationProvider/* = 0*/);
	ImageEditorWindow(CString imageFileName, ConfigurationProvider* configurationProvider/* = 0*/);
	~ImageEditorWindow();
	void setInitialDrawingTool(Canvas::DrawingToolType dt);
	void showUploadButton(bool show);
	void showAddToWizardButton(bool show);
	void setSuggestedFileName(CString string);
	std_tr::shared_ptr<Gdiplus::Bitmap> getResultingBitmap();

	DialogResult DoModal(HWND parent, WindowDisplayMode mode = wdmAuto);

	BEGIN_MSG_MAP(ImageEditorWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER( WM_KEYDOWN, OnKeyDown )
		MESSAGE_HANDLER( WM_KEYUP, OnKeyUp )
		MESSAGE_HANDLER( WM_SIZE, OnSize )
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		MESSAGE_HANDLER( WM_HSCROLL, OnHScroll )
		MESSAGE_HANDLER( MTBM_DROPDOWNCLICKED, OnDropDownClicked )
		MESSAGE_HANDLER( TextParamsWindow::TPWM_FONTCHANGED, OnTextParamWindowFontChanged);
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_RANGE_HANDLER( ID_PEN, ID_MOVE, OnMenuItemClick)
		COMMAND_ID_HANDLER( ID_UNDO, OnUndoClick )
		COMMAND_ID_HANDLER( ID_CLOSE, OnClickedClose )	
		COMMAND_ID_HANDLER( ID_ADDTOWIZARD, OnClickedAddToWizard )	
		COMMAND_ID_HANDLER( ID_UPLOAD, OnClickedUpload )	
		COMMAND_ID_HANDLER( ID_SHARE, OnClickedShare )	
		COMMAND_ID_HANDLER( ID_SAVE, OnClickedSave )	
		COMMAND_ID_HANDLER( ID_SAVEAS, OnClickedSaveAs )	
		MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBackground )
		
		REFLECT_NOTIFICATIONS()
		/*CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)*/
	END_MSG_MAP()
	private:
		ImageEditor::Document* currentDoc_;
		// Handler prototypes (uncomment arguments if needed):
		//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
		//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnKeyUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnDropDownClicked(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnMenuItemClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedAddToWizard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedUpload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedShare(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnClickedSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
		LRESULT OnTextParamWindowFontChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);


		Toolbar horizontalToolbar_;
		Toolbar verticalToolbar_;
		ImageEditor::Canvas* canvas_;
		std::map<int, Canvas::DrawingToolType> menuItems_;
		std::map<Canvas::DrawingToolType, SubMenuItem> subMenuItems_;
		std::map<int,int> selectedSubMenuItems_;
		DialogResult dialogResult_;
		WindowDisplayMode displayMode_;
		Canvas::DrawingToolType initialDrawingTool_;
		bool showUploadButton_;
		bool showAddToWizardButton_;
		CString suggestedFileName_;
		int prevPenSize_;
		CIcon icon_;
		CIcon iconSmall_;
		ColorsDelegate* colorsDelegate_;
		CString sourceFileName_;
		CString outFileName_;
		HWND cropToolTip_;
		int imageQuality_;
		std_tr::shared_ptr<Gdiplus::Bitmap> resultingBitmap_;
		ConfigurationProvider* configurationProvider_; 
		TextParamsWindow textParamsWindow_;
		void createToolbars();
		void OnCropChanged(int x, int y, int w, int h);
		void OnCropFinished(int x, int y, int w, int h);
		void OnDrawingToolChanged(Canvas::DrawingToolType drawingTool);
		void OnTextEditStarted(ImageEditor::TextElement * textElement);
		void OnTextEditFinished(ImageEditor::TextElement * textElement);
		Gdiplus::Bitmap * loadToolbarIcon(int resource);
		void EndDialog(DialogResult dr);
		void init();
		bool saveDocument();
		void updateToolbarDrawingTool(Canvas::DrawingToolType dt);
		void OnForegroundColorChanged(Gdiplus::Color color);
		void OnBackgroundColorChanged(Gdiplus::Color color);
		void onFontChanged(LOGFONT font);
		bool createTooltip();
		void updatePixelLabel();
		void OnSaveAs();
		void saveSettings();
};

class ConfigurationProvider {
public:
	ConfigurationProvider() {
		penSize_ = 12;
	}
	virtual ~ConfigurationProvider() {

	}
	virtual void saveConfiguration() {};
	void setForegroundColor(Gdiplus::Color color) { foregroundColor_ = color; }
	Gdiplus::Color foregroundColor() const { return foregroundColor_; }
	void setBackgroundColor(Gdiplus::Color color) { backgroundColor_ = color; }
	Gdiplus::Color backgroundColor() const { return backgroundColor_; }
	void setPenSize(int size) { penSize_ = size; }
	int penSize() const { return penSize_;}
	void setFont(LOGFONT font) { font_ = font; }
	LOGFONT font() const { return font_; }
protected:
	Gdiplus::Color foregroundColor_;
	Gdiplus::Color backgroundColor_;
	int penSize_;
	LOGFONT font_;
};

}
#endif