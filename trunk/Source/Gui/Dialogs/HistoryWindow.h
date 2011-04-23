/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "../../resource.h"       // main symbols

#include <atlddx.h>
#include <atlframe.h>
#include "../Controls/HistoryTreeControl.h"
#include "../../Func/HistoryManager.h"
#include "../../Gui/Controls/PictureExWnd.h"
// CHistoryWindow

#define ID_OPENINBROWSER 13000
#define ID_COPYTOCLIPBOARD ID_OPENINBROWSER+1
#define ID_VIEWBBCODE ID_OPENINBROWSER +2
#define ID_OPENFOLDER ID_OPENINBROWSER +3 
#define WM_MY_OPENHISTORYFILE WM_USER+101
class CHistoryWindow : public CDialogImpl <CHistoryWindow>,
							public CDialogResize <CHistoryWindow>,
							public CWinDataExchange <CHistoryWindow>,
							public CMessageFilter
{
	public:
		CHistoryWindow();
		~CHistoryWindow();
		enum { IDD = IDD_HISTORYWINDOW };
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		BEGIN_MSG_MAP(CHistoryWindow)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)	
			MESSAGE_HANDLER(WM_MY_OPENHISTORYFILE, OnWmOpenHistoryFile)	
			COMMAND_ID_HANDLER(ID_OPENINBROWSER, OnOpenInBrowser)
			COMMAND_ID_HANDLER(ID_COPYTOCLIPBOARD, OnCopyToClipboard)
			COMMAND_ID_HANDLER(ID_VIEWBBCODE, OnViewBBCode)
			COMMAND_ID_HANDLER(ID_OPENFOLDER, OnOpenFolder)
			COMMAND_HANDLER(IDC_MONTHCOMBO, CBN_SELCHANGE, OnMonthChanged)
			COMMAND_HANDLER(IDC_DOWNLOADTHUMBS, BN_CLICKED, OnDownloadThumbsCheckboxChecked)
			
			CHAIN_MSG_MAP(CDialogResize<CHistoryWindow>)
		//	NOTIFY_HANDLER(IDC_HISTORYTREE, NM_CUSTOMDRAW, OnHistoryTreeCustomDraw)
			REFLECT_NOTIFICATIONS()
			
   //   DEFAULT_REFLECTION_HANDLER()
			//CHAIN_MSG_MAP_MEMBER(m_treeView)
		 END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CHistoryWindow)
			DLGRESIZE_CONTROL(IDC_HISTORYTREE, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_FILESCOUNTLABEL, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_FILESCOUNTDESCR, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_SESSIONSCOUNTLABEL, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_SESSIONSCOUNTDESCR, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_UPLOADTRAFFICDESCR, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_UPLOADTRAFFICLABEL, DLSZ_MOVE_Y)
			
		END_DLGRESIZE_MAP()

		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);	
		LRESULT OnDownloadThumbsCheckboxChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnWmOpenHistoryFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		void Show();
		void FillList(CHistoryReader * mgr);
		CHistoryTreeControl m_treeView;
		CHistoryReader* m_historyReader;
		LRESULT OnHistoryTreeCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		CString m_delayedFileName;
		void threadsStarted();
		void threadsFinished();
		std::vector<CString> m_HistoryFiles;
		bool delayed_closing_;
		CString historyFolder;
		void LoadHistoryFile(CString fileName);
		CPictureExWnd m_wndAnimation;
	//Context menu callbacks
	LRESULT OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyToClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnViewBBCode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnOpenFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);	
	LRESULT OnMonthChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);	
};