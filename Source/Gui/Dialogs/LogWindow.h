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
#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/loglistbox.h"
#include <atlddx.h>
#include <atlframe.h>
#include "Core/Upload/CommonTypes.h"

#define MYWM_WRITELOG WM_USER + 100

// CLogWindow

class CLogWindow : public CDialogImpl <CLogWindow>,
	public CDialogResize <CLogWindow>,
	public CWinDataExchange <CLogWindow>,
	public CMessageFilter
{
	public:
		struct CLogWndMsg
		{
			LogMsgType MsgType;
			LPCWSTR Sender;
			LPCWSTR Msg;
			LPCWSTR Info;
		};

	public:
		CLogWindow();
		~CLogWindow();
		enum { IDD = IDD_LOGWINDOW };
		enum { IDC_CLEARLIST = 12000, IDC_COPYTEXTTOCLIPBOARD};
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		BEGIN_MSG_MAP(CLogWindow)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_ID_HANDLER(IDC_COPYTEXTTOCLIPBOARD, OnCopyToClipboard)
			MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
			MESSAGE_HANDLER(MYWM_WRITELOG, OnWmWriteLog)
			COMMAND_ID_HANDLER(IDC_CLEARLIST, OnClearList)
			CHAIN_MSG_MAP(CDialogResize<CLogWindow>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CLogWindow)
			DLGRESIZE_CONTROL(IDC_MSGLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		END_DLGRESIZE_MAP()

		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		void WriteLog(LogMsgType MsgType, CString Sender, CString Msg, LPCTSTR Info = NULL);
		CLogListBox MsgList;
		void Show();
		LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
		LRESULT OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
		LRESULT OnCopyToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
};

extern CLogWindow LogWindow;

void WriteLog(LogMsgType MsgType, LPCWSTR Sender, LPCWSTR Msg, LPCWSTR Info = NULL);

namespace DefaultErrorHandling {
void ErrorMessage(ErrorInfo);
void DebugMessage(const std::string&, bool);
};

#endif // LOGWINDOW_H
