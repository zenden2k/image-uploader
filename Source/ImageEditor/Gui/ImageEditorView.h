#ifndef IMAGEEDITOR_IMAGEEDITORVIEW_H
#define IMAGEEDITOR_IMAGEEDITORVIEW_H

// ImageEditorView.h : interface of the CImageEditorView class
//
/////////////////////////////////////////////////////////////////////////////
#include <map>
#include "ImageEditor/Canvas.h"
#include <map>
#pragma once

namespace ImageEditor {

class CImageEditorView : public CWindowImpl<CImageEditorView>, public ImageEditor::Canvas::Callback
{
	public:
		struct MenuItem {
			int menuItemId;
			int toolId;
		};
		enum { ID_UNDO = 1000, ID_TEXT = 1001, ID_PEN = 1600, ID_BRUSH, ID_LINE, ID_RECTANGLE,  ID_CROP , ID_SELECTION,};
		DECLARE_WND_CLASS(NULL)
		CImageEditorView();
		BOOL PreTranslateMessage(MSG* pMsg);

		BEGIN_MSG_MAP(CImageEditorView)
			MESSAGE_HANDLER( WM_PAINT, OnPaint )
			MESSAGE_HANDLER( WM_MOUSEMOVE, OnMouseMove )
			MESSAGE_HANDLER( WM_LBUTTONDOWN, OnLButtonDown )
			MESSAGE_HANDLER( WM_LBUTTONUP, OnLButtonUp )
			MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBackground )
			MESSAGE_HANDLER( WM_CONTEXTMENU, OnContextMenu )
			MESSAGE_HANDLER( WM_SETCURSOR, OnSetCursor )
			COMMAND_RANGE_HANDLER( ID_PEN, ID_CROP, OnMenuItemClick)
			COMMAND_ID_HANDLER( ID_UNDO, OnUndoClick )
			COMMAND_ID_HANDLER( ID_TEXT, OnTextClick )

		END_MSG_MAP()

		// Handler prototypes (uncomment arguments if needed):
		//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
		//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
		void setCanvas(ImageEditor::Canvas *canvas);
		void updateView( Canvas* canvas, const CRgn& region );
	protected:
		
		LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnMenuItemClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUndoClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnTextClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		POINT oldPoint;
	private:
		ImageEditor::Canvas *canvas_;
		std::map<int, MenuItem> menuItems_;
		std::map<Canvas::CursorType, HCURSOR> cursorCache_;
		HCURSOR getCachedCursor(Canvas::CursorType cursorType);
};

}
#endif // IMAGEEDITOR_IMAGEEDITORVIEW_H