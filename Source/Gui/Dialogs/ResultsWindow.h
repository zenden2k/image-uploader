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

// ResultsWindow.h : Declaration of the CResultsWindow
// 
// This dialog window shows technical information 
// about  video/audio file that user had selected
#ifndef IU_GUI_RESULTSWINDOW_H
#define IU_GUI_RESULTSWINDOW_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       
#include "ResultsPanel.h"
#include "Gui/Controls/DialogIndirect.h"

class CResultsWindow:	 public CDialogIndirectImpl<CResultsWindow>								
{
	private:
		CWizardDlg *m_WizardDlg;
	public:
		CResultsWindow(CWizardDlg *wizardDlg, CAtlArray<CUrlListItem>  & urlList, bool ChildWindow);
		~CResultsWindow();
		int GetCodeType();
		void UpdateOutput();
		void SetCodeType(int Index);
		void Clear();
		void SetPage(int Index);
		int GetPage();
		void AddServer(ServerProfile  server);
		void InitUpload();
		void FinishUpload();
		void Lock();
		void Unlock();
		void EnableMediaInfo(bool Enable);
		DLGTEMPLATE* GetTemplate();
		void setOnShortenUrlChanged(fastdelegate::FastDelegate1<bool> fd); 
		void setShortenUrls(bool shorten);
		void setUrlList(CAtlArray<CUrlListItem>  * urlList);
		enum { IDD = IDD_RESULTSWINDOW };

	private:
		BEGIN_MSG_MAP(CResultsWindow)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			NOTIFY_HANDLER(IDC_RESULTSTAB, TCN_SELCHANGE, OnTabChanged)
		END_MSG_MAP()
	
	private:
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		CResultsPanel *ResultsPanel;
		bool m_childWindow;
};

#endif // IU_GUI_RESULTSWINDOW_H