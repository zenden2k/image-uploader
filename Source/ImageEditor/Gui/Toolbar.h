#ifndef IMAGEEDITOR_GUI_TOOLBAR_H
#define IMAGEEDITOR_GUI_TOOLBAR_H

#include "atlheaders.h"

#include <vector>

namespace ImageEditor {

class Toolbar : public CWindowImpl<Toolbar> {
public:
	typedef CWindowImpl<Toolbar> TParent;
	enum Orientation { orHorizontal, orVertical };

	struct Item {
		CString title;
		int command;
		CIcon icon;
	};

	Toolbar(Orientation orientation); 
	bool Create(HWND parent);
	int addButton(Item item);
	DECLARE_WND_CLASS(L"ImageEditor_Toolbar");


	BEGIN_MSG_MAP(Toolbar)
		MESSAGE_HANDLER( WM_CREATE, OnCreate )
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		MESSAGE_HANDLER( WM_MOUSEMOVE, OnMouseMove )
		MESSAGE_HANDLER( WM_LBUTTONDOWN, OnLButtonDown )
		MESSAGE_HANDLER( WM_LBUTTONUP, OnLButtonUp )
		MESSAGE_HANDLER( WM_ERASEBKGND, OnEraseBackground )
	END_MSG_MAP()


	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	SIZE CalcItemSize(int index);

protected:
	Orientation orientation_;
	std::vector<Item> buttons_;
};

}
#endif