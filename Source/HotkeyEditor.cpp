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

#include "stdafx.h"
#include "HotkeyEditor.h"

// CHotkeyEditor
CHotkeyEditor::CHotkeyEditor()
{
}

CHotkeyEditor::~CHotkeyEditor()
{
}

MYHOTKEY ConvertHotkeyToPair(WORD HotKey)
{
	MYHOTKEY result;
	result.keyModifier = HIBYTE(HotKey);
	result.keyCode = LOBYTE(HotKey);
	return result;
}

/*WORD ConvertPairToHotkey(MYHOTKEY key )
{
	BYTE newModifiers=0;
	BYTE oldModifiers = key.keyModifier;

	if(oldModifiers&MOD_ALT) newModifiers|=HOTKEYF_ALT;
	if(oldModifiers&MOD_CONTROL) newModifiers|=HOTKEYF_CONTROL;
	if(oldModifiers&MOD_SHIFT) newModifiers|=HOTKEYF_SHIFT;
	
	WORD result;
	result = MAKEWORD(key.keyCode , newModifiers);
	return result;
}*/

LRESULT CHotkeyEditor::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DoDataExchange();
	SetWindowText(m_data.GetDisplayName());
	CenterWindow(GetParent());
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_LOCALHOTKEYLABEL, "Локальная комбинация клавиш:");
	TRC(IDC_GLOBALHOTKEYLABEL, "Глобальная комбинация:");
	TRC(IDC_WINDOWBUTTONCHECKBOX, "Кнопка \"Windows\"");
	
	localHotkeyCtrl.SubclassWindow(GetDlgItem(IDC_LOCALHOTKEY));
	globalHotkeyCtrl.SubclassWindow(GetDlgItem(IDC_GLOBALHOTKEY));
	
	localHotkeyCtrl.SetWinHotkey(m_data.localKey.keyCode,m_data.localKey.keyModifier);
	globalHotkeyCtrl.SetWinHotkey(m_data.globalKey.keyCode,m_data.globalKey.keyModifier);

	SendDlgItemMessage(IDC_WINDOWBUTTONCHECKBOX,BM_SETCHECK,m_data.globalKey.keyModifier&MOD_WIN); 
	return 1;  // Let the system set the focus
}


LRESULT CHotkeyEditor::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	m_data.localKey = ConvertHotkeyToPair(localHotkeyCtrl.GetWinHotkey());
	m_data.globalKey = ConvertHotkeyToPair(globalHotkeyCtrl.GetWinHotkey());
	DoDataExchange(TRUE);
	EndDialog(wID);
	return 0;
}

LRESULT CHotkeyEditor::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
