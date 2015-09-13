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

#include "HotkeySettings.h"

#include "traysettings.h"
#include "hotkeyeditor.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Core/Scripting/API/WebBrowserPrivate_win.h"

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
    TRC(IDC_EDITHOTKEY, "Edit hotkey...");
    TRC(IDC_ATTENTION, "Attention! Global hotkeys are active only when tray icon is shown.");
    GuiTools::MakeLabelBold(GetDlgItem(IDC_ATTENTION));
    m_HotkeyList.AddColumn(TR("Action"),0);
    m_HotkeyList.AddColumn(TR("Local"),1);
    m_HotkeyList.AddColumn(TR("Global"),2);
    CDC hdc = GetDC();
    float dpiScaleX = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
    //float dpiScaleY = GetDeviceCaps(hdc, LOGPIXELSY) / 96.0f;
    m_HotkeyList.SetColumnWidth(0, static_cast<int>(175 * dpiScaleX));
    m_HotkeyList.SetColumnWidth(1, static_cast<int>(82 * dpiScaleX));
    m_HotkeyList.SetColumnWidth(2, static_cast<int>(82 * dpiScaleX));
    m_HotkeyList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
    hotkeyList = Settings.Hotkeys;

    for(int i=0; i < int(hotkeyList.size())-1; i++)
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
    LPNMITEMACTIVATE ia = reinterpret_cast<LPNMITEMACTIVATE>(pnmh);
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
    HWND     hwnd = (HWND) wParam;  
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
    GuiTools::InsertMenu(TrayMenu, 0, IDM_CLEARHOTKEY, TR("Clear")); 
    GuiTools::InsertMenu(TrayMenu, 1, IDM_CLEARALLHOTKEYS, TR("Clear all")); 
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
    for(int i=0; i<int(hotkeyList.size())-1; i++)
    {
        hotkeyList[i+1].Clear();
        m_HotkeyList.SetItem(i, 1, LVIF_TEXT,_T(""),0,0,0,0);
        m_HotkeyList.SetItem(i, 2, LVIF_TEXT,_T(""),0,0,0,0);
    }
    return 0;
}

CHotkeyList::CHotkeyList()
{
#undef TR
#define TR
    m_bChanged = false;
    AddItem(TR("Do nothing"), _T(""), 0);
    AddItem(TR("Tray icon context menu"), _T("contextmenu"), IDM_CONTEXTMENU);
    AddItem(TR("Upload images"),_T("addimages"), IDM_UPLOADIMAGES);
    AddItem(TR("Upload files"),_T("addimages"), IDM_UPLOADFILES);
    AddItem(TR("Upload folder"),_T("addfolder"), IDM_ADDFOLDERS);
    AddItem(TR("Import Video File"),_T("importvideo"), IDM_IMPORTVIDEO);
    AddItem(TR("Screenshot"),_T("screenshotdlg"), IDM_SCREENSHOTDLG);
    AddItem(TR("Capture Rectangular Region"),_T("regionscreenshot"), IDM_REGIONSCREENSHOT);
    AddItem(TR("Capture the Entire Screen"),_T("fullscreenshot"), IDM_FULLSCREENSHOT);
    AddItem(TR("Capture the Active Window"),_T("windowscreenshot"), IDM_WINDOWSCREENSHOT);
    AddItem(TR("Capture Selected Object"),_T("windowhandlescreenshot"), IDM_WINDOWHANDLESCREENSHOT);
    AddItem(TR("Freehand Capture"),_T("freeformscreenshot"), IDM_FREEFORMSCREENSHOT);
    AddItem(TR("Show program's window"),_T("showmainwindow"), IDM_SHOWAPPWINDOW);
    AddItem(TR("Open screenshots folder"), _T("open_screenshot_folder"), IDM_OPENSCREENSHOTSFOLDER);
    AddItem(TR("Settings"),_T("settings"), IDM_SETTINGS);
    AddItem(TR("Paste"),_T("paste"), IDM_PASTEFROMCLIPBOARD,0x56, MOD_CONTROL); // Ctrl+V keyboard shortcut
    AddItem(TR("Images from web"),_T("downloadimages"), IDM_PASTEFROMWEB); // Ctrl+V keyboard shortcut
    AddItem(TR("View Media File Information"),_T("mediainfo"), IDM_MEDIAINFO);
    AddItem(TR("Exit"),_T("mediainfo"), IDM_EXIT);
    AddItem(TR("Shorten a link"),_T("shortenurl"), IDM_SHORTENURL);
    AddItem(TR("Shorten a link from the clipboard"),_T("shortenurlclipboard"), IDM_SHORTENURLCLIPBOARD);
    AddItem(TR("Reupload images"),_T("reuploadimages"), IDM_REUPLOADIMAGES);
}

CHotkeyItem& CHotkeyList::getByFunc(const CString &func)
{
    for(size_t i=0; i<size(); i++)
    {
        if ((*this)[i].func == func) return  (*this)[i];
    }
    return (*this)[0];
}

int CHotkeyList::getFuncIndex(const CString &func)
{
    for(size_t i=0; i<size(); i++)
    {
        if ((*this)[i].func == func) return  i;
    }
    return -1;
}

bool CHotkeyList::operator==( const CHotkeyList& c)
{
    if(size() != c.size()) return false;
    for(size_t i=0; i<c.size(); i++)
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
    push_back(hi);
}

bool CHotkeyList::Changed()
{
    return m_bChanged;
}

CHotkeyList& CHotkeyList::operator=( const CHotkeyList& c)
{
    clear();
    for(size_t i=0; i<c.size(); i++)
    {
        push_back( c[i]);
    }
    m_bChanged = true;
    return *this;
}

CString CHotkeyList::toString() const
{
    CString result;
    for(size_t i=1; i<size(); i++)
    {
        if(!((*this)[i].IsNull()))
        result+=        CString((*this)[i].func) +_T("=")+(*this)[i].localKey.Serialize()+_T(",")+(*this)[i].globalKey.Serialize()+_T(";");
    }
    return result;
}

bool CHotkeyList::DeSerialize(const CString &data)
{
    TCHAR hotkey[200];
    int i =0;
    
    while(WinUtils::ExtractStrFromList(
           data /* Source string */,
            i++, /* Zero based item index */
            hotkey /* Destination buffer */,
            sizeof(hotkey)/sizeof(TCHAR), /* Length in characters of destionation buffer */
            _T(""),
            _T(';')))
    {
        TCHAR funcName[30];
        TCHAR localKeyStr[20],globalKeyStr[20];

        WinUtils::ExtractStrFromList(hotkey , 0, funcName,sizeof(funcName)/sizeof(TCHAR), _T(""),_T('='));
        //(*this)[i].localKey.DeSerialize(localKeyStr);

        int cur = getFuncIndex(funcName);
        if(cur<0) continue;
        
        //(*this)[i].func = funcName;

        WinUtils::ExtractStrFromList(hotkey , 1, funcName,sizeof(funcName)/sizeof(TCHAR), _T(""),_T('='));
        (*this)[cur].localKey.DeSerialize(localKeyStr);

        WinUtils::ExtractStrFromList(funcName , 0, localKeyStr,sizeof(localKeyStr)/sizeof(TCHAR), _T(""),_T(','));
        (*this)[cur].localKey.DeSerialize(localKeyStr);

        WinUtils::ExtractStrFromList(funcName ,1, globalKeyStr ,sizeof(globalKeyStr)/sizeof(TCHAR),_T(""),_T(','));
        (*this)[cur].globalKey.DeSerialize(globalKeyStr);
        
    }
    /* Character to be separator in list */
    return true;
}