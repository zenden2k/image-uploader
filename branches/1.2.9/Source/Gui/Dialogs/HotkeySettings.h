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
#ifndef HOTKEYSETTINGS_H
#define HOTKEYSETTINGS_H


#pragma once

#include "atlheaders.h"
#include <resource.h>       // main symbols
#include <atlcrack.h>
#include <atlctrls.h>
#include "settingspage.h"

#define IDM_CLEARHOTKEY 10000
#define IDM_CLEARALLHOTKEYS (IDM_CLEARHOTKEY + 1)

#include "hotkeyeditor.h"

class CHotkeyItem;
class CHotkeyList: public CAtlArray<CHotkeyItem>
{
	public:
		CHotkeyList();
		bool m_bChanged;
		bool Changed();

		CHotkeyItem& getByFunc(const CString &func);
		void AddItem( CString name, CString func, DWORD commandId, WORD Code=0, WORD modif=0);
		CHotkeyList& operator=( const CHotkeyList& );
		bool operator==( const CHotkeyList& );
		CString toString() const;
		bool DeSerialize(const CString &data);
		int getFuncIndex(const CString &func);
};

class CHotkeySettingsPage : 
	public CDialogImpl<CHotkeySettingsPage>, public CSettingsPage	
{
	public:
		CHotkeySettingsPage();
		~CHotkeySettingsPage();
		bool Apply();
		enum { IDD = IDD_HOTKEYSETTINGSPAGE};
	
	protected:
		 BEGIN_MSG_MAP(CHotkeySettingsPage)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)  
			COMMAND_HANDLER(IDC_EDITHOTKEY, BN_CLICKED, OnEditHotkeyBnClicked)
			COMMAND_HANDLER(IDM_CLEARHOTKEY,BN_CLICKED, OnClearHotkey)
			COMMAND_HANDLER(IDM_CLEARALLHOTKEYS,BN_CLICKED, OnClearAllHotkeys)
			NOTIFY_HANDLER_EX(IDC_HOTKEYLIST, NM_DBLCLK, OnHotkeylistNmDblclk)
			
		 END_MSG_MAP()
		 // Handler prototypes:
		 //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		 //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		 //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClearHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClearAllHotkeys(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnEditHotkeyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		LRESULT OnHotkeylistNmDblclk(LPNMHDR pnmh);
		void EditHotkey(int index);

		CListViewCtrl m_HotkeyList;
		CHotkeyList hotkeyList;
};

#endif // HOTKEYSETTINGS_H


