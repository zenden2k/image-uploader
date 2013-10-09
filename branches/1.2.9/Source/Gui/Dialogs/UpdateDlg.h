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

#ifndef UPDATEDLG_H
#define UPDATEDLG_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Func/UpdatePackage.h"
#include "3rdpart/thread.h"

class CUpdateDlg : 
	public CDialogImpl<CUpdateDlg>,
	public CDialogResize<CUpdateDlg>,
	public CThreadImpl<CUpdateDlg>, 
	public CUpdateStatusCallback
{
	public:
		CUpdateDlg();
		~CUpdateDlg();
		enum { IDD = IDD_UPDATEDLG };

		class CUpdateDlgCallback
		{
			public:
				virtual bool CanShowWindow() = 0;
				virtual void UpdateAvailabilityChanged(bool Available) = 0;
		};
	protected:
		BEGIN_MSG_MAP(CUpdateDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			CHAIN_MSG_MAP(CDialogResize<CUpdateDlg>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CUpdateDlg)
			DLGRESIZE_CONTROL(IDC_UPDATELISTVIEW, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_UPDATEINFO,  DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X| DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		END_DLGRESIZE_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	void setUpdateCallback(CUpdateDlgCallback* callback);
	DWORD Run();
	bool stop;
	bool m_Checked;
	bool m_Modal;
	void CheckUpdates();
	void DoUpdates();
	bool ShowModal(HWND parent);
	void Abort();
	void updateStatus(int packageIndex, const CString& status);

	CListViewCtrl m_listView;
	bool m_bUpdateFinished;
	CUpdateDlgCallback* m_UpdateCallback;
	int m_bClose;
	bool m_InteractiveUpdate;
	CEvent m_UpdateEvent;
	CUpdateManager m_UpdateManager;
};


#endif // UPDATEDLG_H