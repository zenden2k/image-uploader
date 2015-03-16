#ifndef IMAGEEDITOR_GUI_TOOLBAR_H
#define IMAGEEDITOR_GUI_TOOLBAR_H

#include "atlheaders.h"

#include <vector>
#include <GdiPlus.h>

namespace ImageEditor {

class Toolbar : public CWindowImpl<Toolbar> {
public:
	typedef CWindowImpl<Toolbar> TParent;
	enum Orientation { orHorizontal, orVertical };
	enum ItemState { isNormal, isHover, isDown, isDropDown };
	enum ItemType { itButton, itComboButton };
	
	struct Item {
		CString title;
		int command;
		Gdiplus::Bitmap* icon;
		CString hint;
		ItemState state;
		bool checkable;
		RECT rect;
		ItemType type;
		int group;
		bool isChecked;
		Item(CString title, Gdiplus::Bitmap* icon, int command, CString hint = CString(), ItemType type = itButton, bool checkable = false, int group = -1) {
			this->title = title;
			this->icon = icon;
			this->command = command;
			state = isNormal;
			memset(&rect, 0, sizeof(rect));
			this->type = type;
			this->hint = hint;
			this->checkable = checkable;
			isChecked = false;
			group = -1;
		}
	};

	Toolbar(Orientation orientation); 
	~Toolbar();
	bool Create(HWND parent);
	int addButton(Item item);
	DECLARE_WND_CLASS(L"ImageEditor_Toolbar");
	int getItemAtPos(int clientX, int clientY);
	int getItemIndexByCommand(int command);
	void repaintItem(int index);


	BEGIN_MSG_MAP(Toolbar)
		MESSAGE_HANDLER( WM_CREATE, OnCreate )
		MESSAGE_HANDLER( WM_PAINT, OnPaint )
		MESSAGE_HANDLER( WM_MOUSEMOVE, OnMouseMove )
		MESSAGE_HANDLER( WM_MOUSELEAVE, OnMouseLeave )
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
	LRESULT OnMouseLeave(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	SIZE CalcItemSize(int index);
	int AutoSize();

protected:
	Orientation orientation_;
	std::vector<Item> buttons_;
	int selectedItemIndex_;
	void drawItem(int itemIndex, Gdiplus::Graphics* gr, int, int y);
	bool trackMouse_;
	float dpiScaleX;
	float dpiScaleY;
	Gdiplus::Bitmap* dropDownIcon_;
	int itemMargin_;
	int itemHorPadding_;
	int itemVertPadding_;
	int iconSizeX_;
	int iconSizeY_;
	Gdiplus::Font* font_;
	Gdiplus::Color transparentColor_;
};

}
#endif