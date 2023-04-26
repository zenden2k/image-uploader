#ifndef IU_GUI_COMPONENTS_DRAGDNROPOVERLAY_H
#define IU_GUI_COMPONENTS_DRAGDNROPOVERLAY_H

#include <vector>

#include "atlheaders.h"

class CDragndropOverlay :
	public CWindowImpl <CDragndropOverlay>
{
public:
	enum ITEM_ID { kInvalid, kAddToTheList, kImportVideoFile };
	struct Item {
		ITEM_ID id;
		RECT rc;
		CString text;

		Item(ITEM_ID i, const RECT& r, CString t): id(i), rc(r), text(t){
		    
		}
	};

    BEGIN_MSG_MAP(CColorButton)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkg)
    END_MSG_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEraseBkg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void dragMove(int x, int y);
	ITEM_ID itemAtPos(int x, int y);
private:
	CFont font_;
	CDC backBufferDc_;
	CBitmap backBufferBm_;
	HBITMAP oldBm_ = nullptr;
	void updateBackBuffer();
	std::vector<Item> items_;
	int activeItemIndex_ = -1;
};

#endif