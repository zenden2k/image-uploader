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

#ifndef IU_GUI_DIALOGS_NetworkSettingsPage_H
#define IU_GUI_DIALOGS_NetworkSettingsPage_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
#include "settingspage.h"
#include "Gui/WizardCommon.h"
#include "Func/Settings.h"
// CNetworkSettingsPage

class CNetworkSettingsPage : 
	public CDialogImpl<CNetworkSettingsPage>, public CSettingsPage
{
public:
	CNetworkSettingsPage();
virtual ~CNetworkSettingsPage();
	enum { IDD = IDD_NETWORKSETTINGSPAGE };

    BEGIN_MSG_MAP(CNetworkSettingsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_USEPROXYSERVER, BN_CLICKED, OnClickedUseProxy)
		COMMAND_HANDLER(IDC_NEEDSAUTH, BN_CLICKED, OnClickedUseProxyAuth)
	END_MSG_MAP()
		
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedUseProxy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClickedUseProxyAuth(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	virtual bool Apply();
	void TranslateUI();
};

#endif // IU_GUI_DIALOGS_NetworkSettingsPage_H


