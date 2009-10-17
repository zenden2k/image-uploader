/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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
//class CTrayActions;
#include "resource.h"       // main symbols
#include <atlcrack.h>
#include "settingspage.h"
//#include "floatingwindow.h"
// CHotkeySettingsPage



#include "hotkeyeditor.h"
#include <atlctrls.h>
class CHotkeyList: public CAtlArray<CHotkeyItem>
{
public:
	bool m_bChanged;
	bool Changed();

	CHotkeyItem& getByFunc(const CString &func);
	CHotkeyList();
	void AddItem( CString name, CString func, DWORD commandId, WORD Code=0, WORD modif=0);
	CHotkeyList& operator=( const CHotkeyList& );
	bool operator==( const CHotkeyList& );
	CString toString();
	bool DeSerialize(const CString &data);
		
};

class CHotkeySettingsPage : 
	public CDialogImpl<CHotkeySettingsPage>, public CSettingsPage	
{
public:
	CHotkeySettingsPage();
	~CHotkeySettingsPage();
	CHotkeyList hotkeyList;
	enum { IDD = IDD_HOTKEYSETTINGSPAGE};

    BEGIN_MSG_MAP(CHotkeySettingsPage)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
        COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
		  
		COMMAND_HANDLER(IDC_EDITHOTKEY, BN_CLICKED, OnEditHotkeyBnClicked)
		NOTIFY_HANDLER_EX(IDC_HOTKEYLIST, NM_DBLCLK, OnHotkeylistNmDblclk)
		
    END_MSG_MAP()
    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEditHotkeyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//CSavingOptions *so;
	bool Apply();
	CListViewCtrl m_HotkeyList;

	LRESULT OnHotkeylistNmDblclk(LPNMHDR pnmh);
	void EditHotkey(int index);
};


