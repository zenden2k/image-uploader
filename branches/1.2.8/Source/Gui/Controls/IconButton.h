////////////////////////////////////////////////////////////////////////////////
//
// Class CImageButtonWithStyle
//
// The class CIconButton is a CButton dereived class that
// handles the NM_CUSTOMDRAW notification to provide XP visual styles themed
// appearance for buttons with BS_BITMAP or BS_ICON style set.
// This makes them match the appearance of normal buttons with text labels.
// The default Windows controls do not provide the themed appearance for
// these buttons.
//
// To use these buttons, simply subclass the corresponding control with an
// instance of a CIconButton. For buttons on a Dialog, you can
// simple declare CIconButton members for each button in the
// CDialog derived class, and then call DDX_Control() for each instance
// to subclass the corresponding control.
//
//

#pragma once
#include <atlapp.h>
#include <atlbase.h>
#include <atlmisc.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atltheme.h>

class CIconButton : public CWindowImpl<CIconButton>, 
   public CThemeImpl <CIconButton>//, 
    //public CCustomDraw< CIconButton >
{
	public:
		CIconButton();
		virtual ~CIconButton();
		BEGIN_MSG_MAP(CIconButton)  
			REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
		END_MSG_MAP()      

		DECLARE_WND_SUPERCLASS(_T("WTL_CIconButton"), GetWndClassName())
		LRESULT OnCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	private:
		void draw_bitmap (HDC hDC, const CRect& Rect, DWORD style);
		void draw_icon (HDC hDC, const CRect& Rect, DWORD style);
};

