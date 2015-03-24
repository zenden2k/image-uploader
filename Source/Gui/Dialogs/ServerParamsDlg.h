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
#include "resource.h"       // main symbols
#include <atlframe.h>
#include "Func/common.h"
#include "3rdpart/PropertyList.h"
#include "Core/Upload/UploadEngine.h"
#include <Func/Settings.h>
class ServerProfile;
class CServerParamsDlg : 
	public CDialogImpl<CServerParamsDlg>,
	public CDialogResize<CServerParamsDlg>	
{
	public:
		CServerParamsDlg(ServerProfile  serverProfile, bool focusOnLoginEdit = false);
		~CServerParamsDlg();
		enum { IDD = IDD_SERVERPARAMSDLG };

		BEGIN_MSG_MAP(CServerParamsDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_DOAUTH, BN_CLICKED, OnClickedDoAuth)
			COMMAND_HANDLER(IDC_BROWSESERVERFOLDERS, BN_CLICKED, OnBrowseServerFolders)
			COMMAND_HANDLER(IDC_LOGINEDIT, EN_CHANGE, OnLoginEditChange)
			CHAIN_MSG_MAP(CDialogResize<CServerParamsDlg>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CServerParamsDlg)
			DLGRESIZE_CONTROL(IDC_PARAMLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
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
		LRESULT OnClickedDoAuth(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBrowseServerFolders(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnLoginEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		ServerProfile serverProfile();
	protected:
		CPropertyListCtrl m_wndParamList;
		std::map<std::string,std::string> m_paramNameList;
		CUploadEngineData *m_ue;
		bool focusOnLoginControl_;
		CScriptUploadEngine *m_pluginLoader;
		CString oldLogin_;
		ServerProfile  serverProfile_;
		void doAuthChanged();




};


