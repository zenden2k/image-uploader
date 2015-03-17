#ifndef IMAGEEDITOR_MAINFRM_H
#define IMAGEEDITOR_MAINFRM_H

#include "ImageEditor/resource.h"
#include "ImageEditorView.h"
#include "ImageEditor/Document.h"
#include "Toolbar.h"
// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
namespace ImageEditor {

class ImageEditorWindow : public CWindowImpl<ImageEditorWindow>
{
public:
	DECLARE_WND_CLASS(_T("CMainFrame"))
	enum { ID_UNDO = 1000, ID_TEXT = 1001,  ID_COLORPICKER,  ID_PEN = 1600, ID_BRUSH, ID_LINE, ID_ARROW, ID_RECTANGLE, 
		ID_FILLEDRECTANGLE, ID_CROP , ID_SELECTION,ID_MOVE};
	struct MenuItem {
		int menuItemId;
		int toolId;
	};

	ImageEditor::CImageEditorView m_view;

	ImageEditorWindow();
	~ImageEditorWindow();

	BEGIN_MSG_MAP(ImageEditorWindow)
		if (uMsg == WM_COMMAND && EN_CHANGE == HIWORD(wParam)) \
		{ 
			//SetMsgHandled(TRUE); 

		}
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER( WM_KEYDOWN, OnKeyDown )
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_RANGE_HANDLER( ID_PEN, ID_MOVE, OnMenuItemClick)
		COMMAND_ID_HANDLER( ID_UNDO, OnUndoClick )
		COMMAND_ID_HANDLER( ID_TEXT, OnTextClick )
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
		LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnMenuItemClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		Toolbar horizontalToolbar_;
		Toolbar verticalToolbar_;
		CImageList toolbarImageList_;
		ImageEditor::Canvas* canvas_;
		std::map<int, MenuItem> menuItems_;
		void createToolbars();
		void OnCropChanged(int x, int y, int w, int h);
		Gdiplus::Bitmap * loadToolbarIcon(int resource);
};

}
#endif