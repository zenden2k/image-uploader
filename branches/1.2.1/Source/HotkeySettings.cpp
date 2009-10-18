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

#include "stdafx.h"
#include "HotkeySettings.h"
#include "traysettings.h"
#include "hotkeyeditor.h"

CHotkeyList::CHotkeyList()
{
	m_bChanged = false;
	AddItem(TR("Контекстное меню значка"), _T(""), IDM_CONTEXTMENU);
	AddItem(TR("Загрузить файлы"),_T("addimages"), IDM_UPLOADFILES);
	AddItem(TR("Загрузить папку"),_T("addfolder"), IDM_ADDFOLDER);
	AddItem(TR("Импорт видео"),_T("importvideo"), IDM_IMPORTVIDEO);
	AddItem(TR("Скриншот"),_T("screenshotdlg"), IDM_SCREENSHOTDLG);
	AddItem(TR("Скриншот выделенной области"),_T("regionscreenshot"), IDM_REGIONSCREENSHOT);
	AddItem(TR("Скриншот всего экрана"),_T("fullscreenshot"), IDM_FULLSCREENSHOT);
	AddItem(TR("Скриншот текущего окна"),_T("windowscreenshot"), IDM_FULLSCREENSHOT);
	AddItem(TR("Показать окно программы"),_T("-"), IDM_SHOWAPPWINDOW);
	AddItem(TR("Настройки"),_T("settings"), IDM_SETTINGS);
	AddItem(TR("Вставить из буфера"),_T("paste"), IDM_PASTEFROMCLIPBOARD,/*VkKeyScan('V')*/0x56, MOD_CONTROL);
	AddItem(TR("Информация о медиафайле"),_T("mediainfo"), IDM_MEDIAINFO);
}

CHotkeyItem& CHotkeyList::getByFunc(const CString &func)
{
	for(int i=0; i<GetCount(); i++)
	{
		if ((*this)[i].func == func) return  (*this)[i];
	}
}

bool CHotkeyList::operator==( const CHotkeyList& c)
{
	if(GetCount() != c.GetCount()) return false;
	for(int i=0; i<c.GetCount(); i++)
	{
		if((*this)[i].localKey!=c[i].localKey || (*this)[i].globalKey!=c[i].globalKey)
				return false;

	}
	return false;
}

void CHotkeyList::AddItem(CString name, CString func, DWORD commandId, WORD code, WORD modif)
{
	CHotkeyItem hi;
	hi.localKey.keyCode = code;
	hi.localKey.keyModifier = modif;

	hi.func = func;
	hi.name = name;
	hi.commandId = commandId;
	Add(hi);
}

bool CHotkeyList::Changed()
{
	return m_bChanged;
}

CHotkeyList& CHotkeyList::operator=( const CHotkeyList& c)
{
	RemoveAll();
	for(int i=0; i<c.GetCount(); i++)
	{
		Add( c[i]);
	}
	m_bChanged = true;
	return *this;
}

CString CHotkeyList::toString()
{
	CString result;
	for(int i=0; i<GetCount(); i++)
	{
		result+=		(*this)[i].localKey.Serialize()+_T(",")+(*this)[i].globalKey.Serialize()+_T(";");
	}
	//MessageBox(0, result,0,0);
	return result;
}

bool CHotkeyList::DeSerialize(const CString &data)
{
	TCHAR hotkey[200];
	int i =0;
	
	while(ExtractStrFromList(
           data /* Source string */,
            i, /* Zero based item index */
            hotkey /* Destination buffer */,
            sizeof(hotkey), /* Length in characters of destionation buffer */
            _T(""),
            _T(';')))
	{
		
		TCHAR localKeyStr[20],globalKeyStr[20];
		ExtractStrFromList(
           hotkey /* Source string */,
            0, /* Zero based item index */
           localKeyStr /* Destination buffer */,
            sizeof(localKeyStr), /* Length in characters of destionation buffer */
            _T(""),
            _T(','));
		(*this)[i].localKey.DeSerialize(localKeyStr);
		ExtractStrFromList(
           hotkey /* Source string */,
            1, /* Zero based item index */
            globalKeyStr /* Destination buffer */,
            sizeof(globalKeyStr), /* Length in characters of destionation buffer */
            _T(""),
            _T(','));
		(*this)[i].globalKey.DeSerialize(globalKeyStr);
		i++;
	}
	/* Character to be separator in list */
	return true;
}
// CHotkeySettingsPage
CHotkeySettingsPage::CHotkeySettingsPage()
{

}

CHotkeySettingsPage::~CHotkeySettingsPage()
{
}

LRESULT CHotkeySettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);
	m_HotkeyList.m_hWnd = GetDlgItem(IDC_HOTKEYLIST);
	TRC(IDC_EDITHOTKEY, "Редактировать...");
	TRC(IDC_ATTENTION, "Внимание! Глобальные сочетания действуют, только если включена опция \"Показывать иконку в трее\".");
	m_HotkeyList.AddColumn(TR("Действие"),0);
	m_HotkeyList.AddColumn(TR("Локальные"),1);
	m_HotkeyList.AddColumn(TR("Глобальные"),2);
	m_HotkeyList.SetColumnWidth(0,190);
	m_HotkeyList.SetColumnWidth(1,90);
	m_HotkeyList.SetColumnWidth(2,90);
	m_HotkeyList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	hotkeyList = Settings.Hotkeys;

	for(int i=0; i< hotkeyList.GetCount(); i++)
	{
		m_HotkeyList.AddItem(i,0,Lang.GetString(hotkeyList[i].name));		
		m_HotkeyList.AddItem(i,1,hotkeyList[i].localKey.toString());
		m_HotkeyList.AddItem(i,2,hotkeyList[i].globalKey.toString());
	}
	return 1;  // Let the system set the focus
}

LRESULT CHotkeySettingsPage::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}

LRESULT CHotkeySettingsPage::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	EndDialog(wID);
	return 0;
}
bool CHotkeySettingsPage::Apply()
{
	if(!(Settings.Hotkeys == hotkeyList))
		Settings.Hotkeys_changed = true;
	Settings.Hotkeys = hotkeyList;
	return true;
}

LRESULT CHotkeySettingsPage::OnHotkeylistNmDblclk(LPNMHDR pnmh)
{
	LPNMITEMACTIVATE ia = (LPNMITEMACTIVATE)pnmh;
	if(ia)
		EditHotkey(ia->iItem);

	return 0;
}

void CHotkeySettingsPage::EditHotkey(int index)
{
	if(index < 0) return ;

	CHotkeyEditor hotKeyDlg;
	hotKeyDlg.m_data = hotkeyList[index];
	if(hotKeyDlg.DoModal(m_hWnd)==IDOK)
	{
		hotkeyList[index] = hotKeyDlg.m_data ;
		m_HotkeyList.AddItem(index,1,hotKeyDlg.m_data.localKey.toString());
		m_HotkeyList.AddItem(index,2,hotKeyDlg.m_data.globalKey.toString());
	}
}

LRESULT CHotkeySettingsPage::OnEditHotkeyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int index = m_HotkeyList.GetSelectedIndex();
	EditHotkey(index);
	return 0;
}
