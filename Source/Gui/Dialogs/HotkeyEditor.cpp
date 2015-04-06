/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

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
	m_data.localKey = ConvertHotkeyToPair(static_cast<WORD>(localHotkeyCtrl.GetWinHotkey()));
	m_data.globalKey =ConvertHotkeyToPair( static_cast<WORD>(globalHotkeyCtrl.GetWinHotkey()));
	DoDataExchange(TRUE);
	EndDialog(wID);
	return 0;
}

LRESULT CHotkeyEditor::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
