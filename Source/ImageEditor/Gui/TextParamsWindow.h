/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include "atlheaders.h"
#include "Gui/Controls/MyImage.h"
#include "resource.h"

namespace ZThread {
	class Thread;
}
class FontEnumerator;

class TextParamsWindow : public CDialogImpl<TextParamsWindow>
{
	typedef CDialogImpl<TextParamsWindow> TBase;
	public:
		enum { IDD = IDD_TEXTPARAMSWINDOW, TPWM_FONTCHANGED = WM_USER + 123, IDC_BOLD = 1500, IDC_ITALIC, IDC_UNDERLINE};
		TextParamsWindow();
		~TextParamsWindow();
		void setFont(LOGFONT logFont);
		LOGFONT getFont();
		/*HWND Create(HWND parent);*/
	protected:
		BEGIN_MSG_MAP(CTextParamsWindow)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			COMMAND_HANDLER(IDC_FONTCOMBO, CBN_SELCHANGE, OnFontSelChange);
			COMMAND_HANDLER(IDC_FONTSIZECOMBO, CBN_SELCHANGE, OnFontSizeSelChange);
			COMMAND_ID_HANDLER(IDC_BOLD, OnBoldClick)
			COMMAND_ID_HANDLER(IDC_ITALIC, OnItalicClick)
			COMMAND_ID_HANDLER(IDC_UNDERLINE, OnUnderlineClick)
		END_MSG_MAP()

		// Handler prototypes (uncomment arguments if needed):
		//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
		//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
		//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

		// Message handlers
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		LRESULT OnFontSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnFontSizeSelChange(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnBoldClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnItalicClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		LRESULT OnUnderlineClick(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		CToolBarCtrl textToolbar_;
		void OnFontEnumerationFinished();
		ZThread::Thread * fontEnumerationThread_;
		CComboBox fontComboBox_;
		CComboBox fontSizeComboBox_;
		CDC dc_;
		CString fontName_;
		LOGFONT font_;
		CImageList toolbarImageList_;
		void NotifyParent(DWORD changeMask);
		std::vector<LOGFONT> fonts_;
};
