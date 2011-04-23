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

#include "../../atlheaders.h"
#include "HotkeySettings.h"
#include "traysettings.h"
#include "hotkeyeditor.h"
 
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
	m_HotkeyList.SetColumnWidth(0,185);
	m_HotkeyList.SetColumnWidth(1,87);
	m_HotkeyList.SetColumnWidth(2,87);
	m_HotkeyList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
	hotkeyList = Settings.Hotkeys;

	for(int i=0; i < int(hotkeyList.GetCount())-1; i++)
	{
		m_HotkeyList.AddItem(i, 0, hotkeyList[i+1].GetDisplayName());		
		m_HotkeyList.AddItem(i, 1, hotkeyList[i+1].localKey.toString());
		m_HotkeyList.AddItem(i, 2, hotkeyList[i+1].globalKey.toString());
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
	hotKeyDlg.m_data = hotkeyList[index+1];
	if(hotKeyDlg.DoModal(m_hWnd)==IDOK)
	{
		hotkeyList[index+1] = hotKeyDlg.m_data ;
		m_HotkeyList.AddItem(index,1,hotKeyDlg.m_data.localKey.toString());
		m_HotkeyList.AddItem(index,2,hotKeyDlg.m_data.globalKey.toString());
	}
}

LRESULT CHotkeySettingsPage::OnEditHotkeyBnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int index = m_HotkeyList.GetSelectedIndex();
	if(index>=0)
		EditHotkey(index);
	return 0;
}

LRESULT CHotkeySettingsPage::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND 	hwnd = (HWND) wParam;  
	POINT ClientPoint, ScreenPoint;

	if(lParam == -1) 
	{
		ClientPoint.x = 0;
		ClientPoint.y = 0;
		ScreenPoint = ClientPoint;
		::ClientToScreen(hwnd, &ScreenPoint);
	}
	else
	{
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}
	HMENU TrayMenu = ::CreatePopupMenu();
	IUInsertMenu(TrayMenu, 0, IDM_CLEARHOTKEY, TR("Очистить")); 
	IUInsertMenu(TrayMenu, 1, IDM_CLEARALLHOTKEYS, TR("Очистить всё")); 
	::TrackPopupMenu(TrayMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, 0,m_hWnd,0);
	return 0;
}


LRESULT CHotkeySettingsPage::OnClearHotkey(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int index = m_HotkeyList.GetSelectedIndex();
	if(index)
	{
		hotkeyList[index+1].Clear();
		m_HotkeyList.SetItem(index, 1, LVIF_TEXT,_T(""),0,0,0,0);
		m_HotkeyList.SetItem(index, 2, LVIF_TEXT,_T(""),0,0,0,0);
	}
	return 0;
}

LRESULT CHotkeySettingsPage::OnClearAllHotkeys(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	for(size_t i=0; i<hotkeyList.GetCount()-1; i++)
	{
		hotkeyList[i+1].Clear();
		m_HotkeyList.SetItem(i, 1, LVIF_TEXT,_T(""),0,0,0,0);
		m_HotkeyList.SetItem(i, 2, LVIF_TEXT,_T(""),0,0,0,0);
	}
	return 0;
}




CHotkeyList::CHotkeyList()
{
	m_bChanged = false;
	AddItem(TR("Нет действия"), _T(""), 0);
	AddItem(TR("Контекстное меню значка"), _T("contextmenu"), IDM_CONTEXTMENU);
	AddItem(TR("Загрузить изображения"),_T("addimages"), IDM_UPLOADIMAGES);
	AddItem(TR("Загрузить файлы"),_T("addimages"), IDM_UPLOADFILES);
	AddItem(TR("Загрузить папку"),_T("addfolder"), IDM_ADDFOLDER);
	AddItem(TR("Импорт видео"),_T("importvideo"), IDM_IMPORTVIDEO);
	AddItem(TR("Скриншот"),_T("screenshotdlg"), IDM_SCREENSHOTDLG);
	AddItem(TR("Снимок прямоугольной области"),_T("regionscreenshot"), IDM_REGIONSCREENSHOT);
	AddItem(TR("Снимок всего экрана"),_T("fullscreenshot"), IDM_FULLSCREENSHOT);
	AddItem(TR("Снимок активного окна"),_T("windowscreenshot"), IDM_WINDOWSCREENSHOT);
	AddItem(TR("Снимок выбранного элемента"),_T("windowhandlescreenshot"), IDM_WINDOWHANDLESCREENSHOT);
	AddItem(TR("Снимок произвольной формы"),_T("freeformscreenshot"), IDM_WINDOWSCREENSHOT);
	AddItem(TR("Показать окно программы"),_T("showmainwindow"), IDM_SHOWAPPWINDOW);
	AddItem(TR("Настройки"),_T("settings"), IDM_SETTINGS);
	AddItem(TR("Вставить из буфера"),_T("paste"), IDM_PASTEFROMCLIPBOARD,0x56, MOD_CONTROL); // Ctrl+V keyboard shortcut
	AddItem(TR("Изображения из Web"),_T("downloadimages"), IDM_PASTEFROMWEB); // Ctrl+V keyboard shortcut
	AddItem(TR("Информация о медиафайле"),_T("mediainfo"), IDM_MEDIAINFO);
	AddItem(TR("Выход"),_T("mediainfo"), IDM_EXIT);
}

CHotkeyItem& CHotkeyList::getByFunc(const CString &func)
{
	for(size_t i=0; i<GetCount(); i++)
	{
		if ((*this)[i].func == func) return  (*this)[i];
	}
	return (*this)[0];
}

int CHotkeyList::getFuncIndex(const CString &func)
{
	for(size_t i=0; i<GetCount(); i++)
	{
		if ((*this)[i].func == func) return  i;
	}
	return -1;
}

bool CHotkeyList::operator==( const CHotkeyList& c)
{
	if(GetCount() != c.GetCount()) return false;
	for(size_t i=0; i<c.GetCount(); i++)
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
	for(size_t i=0; i<c.GetCount(); i++)
	{
		Add( c[i]);
	}
	m_bChanged = true;
	return *this;
}

CString CHotkeyList::toString() const
{
	CString result;
	for(size_t i=1; i<GetCount(); i++)
	{
		if(!((*this)[i].IsNull()))
		result+=		CString((*this)[i].func) +_T("=")+(*this)[i].localKey.Serialize()+_T(",")+(*this)[i].globalKey.Serialize()+_T(";");
	}
	return result;
}

bool CHotkeyList::DeSerialize(const CString &data)
{
	TCHAR hotkey[200];
	int i =0;
	
	while(ExtractStrFromList(
           data /* Source string */,
            i++, /* Zero based item index */
            hotkey /* Destination buffer */,
            sizeof(hotkey)/sizeof(TCHAR), /* Length in characters of destionation buffer */
            _T(""),
            _T(';')))
	{
		TCHAR funcName[30];
		TCHAR localKeyStr[20],globalKeyStr[20];

		ExtractStrFromList(hotkey , 0, funcName,sizeof(funcName)/sizeof(TCHAR), _T(""),_T('='));
		//(*this)[i].localKey.DeSerialize(localKeyStr);

		int cur = getFuncIndex(funcName);
		if(cur<0) continue;
		
		//(*this)[i].func = funcName;

		ExtractStrFromList(hotkey , 1, funcName,sizeof(funcName)/sizeof(TCHAR), _T(""),_T('='));
		(*this)[cur].localKey.DeSerialize(localKeyStr);

		ExtractStrFromList(funcName , 0, localKeyStr,sizeof(localKeyStr)/sizeof(TCHAR), _T(""),_T(','));
		(*this)[cur].localKey.DeSerialize(localKeyStr);
		
		ExtractStrFromList(funcName ,1, globalKeyStr ,sizeof(globalKeyStr)/sizeof(TCHAR),_T(""),_T(','));
		(*this)[cur].globalKey.DeSerialize(globalKeyStr);
		
	}
	/* Character to be separator in list */
	return true;
}