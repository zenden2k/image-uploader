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
#ifndef ContextMenuItemDlg_H
#define ContextMenuItemDlg_H

#pragma once
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/Upload/UploadEngine.h"
#include <Gui/Controls/ServerSelectorControl.h>
class ServerProfile;
class CServerSelectorControl;
// CContextMenuItemDlg


class CContextMenuItemDlg : public CDialogImpl<CContextMenuItemDlg>	
{
	public:
		int ServerId;
		CContextMenuItemDlg();
		~CContextMenuItemDlg();
		enum { IDD = IDD_CONTEXTMENUITEMDLG };
	protected:
		BEGIN_MSG_MAP(CContextMenuItemDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			MESSAGE_HANDLER(WM_SERVERSELECTCONTROL_CHANGE, OnServerSelectControlChanged)
			COMMAND_HANDLER(IDC_MENUITEMTITLEEDIT, EN_CHANGE, OnMenuItemTitleEditChange);
			
		END_MSG_MAP()
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnServerSelectControlChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnMenuItemTitleEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		ServerProfile serverProfile();
		CString menuItemTitle();
protected:
	CServerSelectorControl *imageServerSelector_;
	ServerProfile serverProfile_;
	bool titleEdited_;
	CString title_;
	void generateTitle();
};

#endif // ContextMenuItemDlg_H


