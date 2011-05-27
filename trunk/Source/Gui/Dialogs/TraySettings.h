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

#ifndef TRAYSETTINGS_H
#define TRAYSETTINGS_H

#pragma once

#include <atlcrack.h>
#include "../../resource.h"       // main symbols

#include "floatingwindow.h"
// CTraySettingsPage

struct TrayItem
{
	CString text;
	DWORD commandId;
};

class CTrayActions: public CAtlArray<TrayItem>
{
	public:
	CTrayActions();
	void AddTrayAction(CString text, DWORD id)
	{
		TrayItem item;
		item.text = text;
		item.commandId = id;
		Add(item);
	}
};


class CTraySettingsPage : 
	public CDialogImpl<CTraySettingsPage>, public CSettingsPage	
{
public:
	CTraySettingsPage();
	~CTraySettingsPage();
	enum { IDD = IDD_TRAYSETTINGSPAGE};

    BEGIN_MSG_MAP(CTraySettingsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER_EX(IDC_SHOWTRAYICON, BN_CLICKED, OnShowTrayIconBnClicked)
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	bool Apply();
	LRESULT OnShowTrayIconBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl);
};



#endif // TRAYSETTINGS_H