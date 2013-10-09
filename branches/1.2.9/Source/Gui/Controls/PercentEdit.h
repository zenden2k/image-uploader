
#pragma once
#include <atlapp.h>
#include <atlbase.h>
#include <atlmisc.h>
#include <atlcrack.h>
#include <atlctrls.h>

class CPercentEdit : public CWindowImpl<CPercentEdit>
{
	public:
		CPercentEdit();
		virtual ~CPercentEdit();
		BEGIN_MSG_MAP(CPercentEdit)  
			REFLECTED_COMMAND_CODE_HANDLER( EN_KILLFOCUS, OnKillFocus)
			REFLECTED_COMMAND_CODE_HANDLER( EN_SETFOCUS, OnSetFocus)
			MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
			MESSAGE_HANDLER(WM_GETTEXT, OnGetText)
	  END_MSG_MAP()      
	BOOL SubclassWindow(HWND hWnd);
	DECLARE_WND_SUPERCLASS(_T("WTL_CPercentEdit"), GetWndClassName())
	LRESULT OnKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSetFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnGetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void setUnit(const CString& text);
	protected:
		CString unit_str_;
		void Init();
		CString getCurrentPostfix();
};

