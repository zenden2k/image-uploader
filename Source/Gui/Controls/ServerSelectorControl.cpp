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

#include "ServerSelectorControl.h"
#include <uxtheme.h>

#include "Gui/Dialogs/wizarddlg.h"
#include "Gui/GuiTools.h"
#include "Func/Common.h"
#include "Func/MyUtils.h"
#include "Gui/Dialogs/ServerParamsDlg.h"
#include "Gui/Dialogs/UploadParamsDlg.h"
#include "Func/WinUtils.h"
#include "Gui/IconBitmapUtils.h"
#include "Gui/Dialogs/LoginDlg.h"
#include "Gui/Dialogs/AddFtpServerDialog.h"
#include "Gui/Dialogs/AddDirectoryServerDialog.h"
#include "Core/Logging.h"

const char CServerSelectorControl::kAddFtpServer[]=("<add_ftp_server>");
const char CServerSelectorControl::kAddDirectoryAsServer[]=("<add_directory_as_server>");
const TCHAR MENU_EXIT_NOTIFY[] = _T("MENU_EXIT_NOTIFY"), MENU_EXIT_COMMAND_ID[] = _T("MENU_EXIT_COMMAND_ID");
// CServerSelectorControl
CServerSelectorControl::CServerSelectorControl(UploadEngineManager* uploadEngineManager, bool defaultServer, bool isChildWindow)
{
        showDefaultServerItem_ = false;
        serversMask_ = smImageServers | smFileServers;
        showImageProcessingParams_ = true;
        showParamsLink_ = true;
        defaultServer_ = defaultServer;
        iconBitmapUtils_ = new IconBitmapUtils();
        previousSelectedServerIndex = -1;
        uploadEngineManager_ = uploadEngineManager;
        isChildWindow_ = isChildWindow;
}

CServerSelectorControl::~CServerSelectorControl()
{
    delete iconBitmapUtils_;
}

void CServerSelectorControl::TranslateUI() {
}
    
LRESULT CServerSelectorControl::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TranslateUI();

    imageProcessingParamsLink_.SubclassWindow(GetDlgItem(IDC_IMAGEPROCESSINGPARAMS));
    imageProcessingParamsLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON; 
    imageProcessingParamsLink_.m_clrLink = CSettings::DefaultLinkColor;
    CString linkLabel = showImageProcessingParams_ ? TR("Image processing settings...") : TR("Settings...");
    imageProcessingParamsLink_.SetLabel(linkLabel);
    imageProcessingParamsLink_.SetToolTipText(linkLabel);
    imageProcessingParamsLink_.ShowWindow(showParamsLink_ ? SW_SHOW : SW_HIDE);
    accountLink_.SubclassWindow(GetDlgItem(IDC_ACCOUNTINFO));
    accountLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON ; 
    accountLink_.m_clrLink = CSettings::DefaultLinkColor;
    accountLink_.SetToolTipText(TR("User name"));

    createSettingsButton();
    setTitle(title_);
    GuiTools::MakeLabelBold( GetDlgItem( IDC_SERVERGROUPBOX) );
    CIcon deleteIcon = LoadIcon(GetModuleHandle(0),MAKEINTRESOURCE(IDI_ICONDELETE));
    serverComboBox_.Attach( GetDlgItem( IDC_SERVERCOMBOBOX ) );

    updateServerList();
    //GuiTools::ShowDialogItem(m_hWnd, IDC_IMAGEPROCESSINGPARAMS, showImageProcessingParams_);

    return FALSE;  
}

LRESULT CServerSelectorControl::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    int count = serverComboBox_.GetCount();
    for (int i = 0; i < count; i++)
    {
        char * data = reinterpret_cast<char*>(serverComboBox_.GetItemDataPtr(i));
        if (data && *data != '<')
        {
            delete[] data;
        }
        
    }

    return 0;
}

void CServerSelectorControl::setTitle(CString title) {
    if (m_hWnd)
    {
        SetDlgItemText(IDC_SERVERGROUPBOX, title);
    }
    title_ = title;
}

void CServerSelectorControl::setServerProfile(ServerProfile serverProfile) {
    serverProfile_ = serverProfile;

    if ( !m_hWnd ) {
        return;
    }

    int comboboxItemIndex = serverComboBox_.FindStringExact(-1, Utf8ToWCstring(serverProfile.serverName()));

    if ( comboboxItemIndex == CB_ERR) {
        serverComboBox_.SetCurSel(0); //random server
    } else {
        serverComboBox_.SetCurSel(comboboxItemIndex);
    }
    previousSelectedServerIndex = comboboxItemIndex;
    updateInfoLabel();
}

ServerProfile CServerSelectorControl::serverProfile() const {
    return serverProfile_;
}

LRESULT CServerSelectorControl::OnClickedEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CServerParamsDlg serverParamsDlg(serverProfile_, uploadEngineManager_);
    if (serverParamsDlg.DoModal(isChildWindow_ ? m_hWnd : GetParent()) == IDOK) {
        serverProfile_ = serverParamsDlg.serverProfile();
        notifyChange();
        
    }

    updateInfoLabel();
    return 0;
}

LRESULT CServerSelectorControl::OnServerComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    serverChanged();
    return 0;
}

void CServerSelectorControl::addAccount()
{
    ServerProfile serverProfileCopy = serverProfile_;
    serverProfileCopy.setProfileName("");
    CLoginDlg dlg(serverProfileCopy, uploadEngineManager_, true);

    if (dlg.DoModal(isChildWindow_ ? m_hWnd : GetParent()) == IDOK)
    {
        serverProfileCopy.setProfileName(WCstringToUtf8(dlg.accountName()));
        serverProfileCopy.setFolderId("");
        serverProfileCopy.setFolderTitle("");
        serverProfileCopy.setFolderUrl("");

        serverProfile_ = serverProfileCopy;
        updateInfoLabel();
        notifyChange();
    }
}

void CServerSelectorControl::serverChanged() {
    //return;
    CUploadEngineData * uploadEngineData =  0;
    CString profileName;
    int serverComboElementIndex = serverComboBox_.GetCurSel();
    char *lpstrServerName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );

    if ( !lpstrServerName ) {
        serverComboElementIndex++;
        serverComboBox_.SetCurSel(serverComboElementIndex);
        lpstrServerName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
    }
    if ( serverComboElementIndex > 0 && lpstrServerName ) {
        std::string serverName = lpstrServerName;
        CString serverNameW = Utf8ToWCstring( serverName );
        serverProfile_.setServerName(serverName);
        if ( serverName == kAddFtpServer ) {
            CAddFtpServerDialog dlg(_EngineList);
            if ( dlg.DoModal(m_hWnd) == IDOK ) {
                serverProfile_ = ServerProfile();
                serverProfile_.setServerName(WCstringToUtf8(dlg.createdServerName()));
                serverProfile_.setProfileName(WCstringToUtf8(dlg.createdServerLogin()));
                serverProfile_.clearFolderInfo();
                notifyServerListChanged();

            } else {
                serverComboBox_.SetCurSel(previousSelectedServerIndex);
                serverChanged();
                return;
            }
        } else if (serverName == kAddDirectoryAsServer  ) {
            CAddDirectoryServerDialog dlg(_EngineList);
            if ( dlg.DoModal(m_hWnd) == IDOK ) {
                serverProfile_ = ServerProfile();
                serverProfile_.setServerName(WCstringToUtf8(dlg.createdServerName()));
                serverProfile_.setProfileName("");
                serverProfile_.clearFolderInfo();
                notifyServerListChanged();
            } else {
                serverComboBox_.SetCurSel(previousSelectedServerIndex);
                serverChanged();
                return;
            }
        } else if ( serverName != CMyEngineList::DefaultServer && serverName != CMyEngineList::RandomServer ) {
            uploadEngineData = _EngineList->byName( serverNameW );
            if ( !uploadEngineData ) {
                return ;
            }

        
            //std::map<CString,ServerSettingsStruct> serverProfiles;
            //serverProfiles[serverNameW]= Settings.ServersSettings[serverName];
        
        //    ShowVar((int)Settings.ServersSettings[serverName].size());
            
            if ( Settings.ServersSettings[serverName].size() ) {


                std::map <std::string, ServerSettingsStruct>& ss = Settings.ServersSettings[serverName];
                std::map <std::string, ServerSettingsStruct>::iterator it = ss.begin();
                if ( it->first == "" ) {
                    ++it;
                }
                if ( it!= ss.end() ) {
                    ServerSettingsStruct & s = it->second;
                    serverProfile_.setProfileName(s.authData.Login);
                    serverProfile_.setFolderId(s.defaultFolder.getId());
                    serverProfile_.setFolderTitle(s.defaultFolder.getTitle());
                    serverProfile_.setFolderUrl(s.defaultFolder.viewUrl);
                } else {
                    serverProfile_.setProfileName("");
                    serverProfile_.setFolderId("");
                    serverProfile_.setFolderTitle("");
                    serverProfile_.setFolderUrl("");
                }
                
            }
        }

    
        
    }
    previousSelectedServerIndex = serverComboElementIndex;
    notifyChange();

    updateInfoLabel();
}

void CServerSelectorControl::updateInfoLabel() {
    int serverComboElementIndex = serverComboBox_.GetCurSel();
    std::string serverName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
    currentUserName_.Empty();

    bool showServerParams = (serverName != CMyEngineList::DefaultServer && serverName != CMyEngineList::RandomServer );

    if (  !showServerParams ) {
        GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, showServerParams );
        GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERICON, showServerParams );
        GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showServerParams );
        GuiTools::ShowDialogItem(m_hWnd, IDC_USERICON, showServerParams );
        settingsButtonToolbar_.ShowWindow(SW_HIDE);
        return;
    }
    //GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showServerParams);
//    GuiTools::ShowDialogItem(m_hWnd, IDC_EDIT, showServerParams);

    CUploadEngineData* uploadEngineData = _EngineList->byName(Utf8ToWCstring( serverName ));
    if ( ! uploadEngineData ) {
        return;
    }

    if ( uploadEngineData ) {
        showServerParams = showServerParams && (uploadEngineData->UsingPlugin || uploadEngineData->NeedAuthorization);
    }


    CString accountInfoText;// = TR("Prof:") + serverProfile_.profileName() + _T(" ");
    LoginInfo loginInfo = serverProfile_.serverSettings().authData;
    //accountInfoText += TR("Account:") + CString(" ");
    
    if ( loginInfo.Login.empty() || (!loginInfo.DoAuth  && uploadEngineData->NeedAuthorization != 2 ) ) {
        accountInfoText += TR("Account...");
        accountLink_.SetToolTipText(TR("Enter account information"));
    } else {
        accountInfoText += Utf8ToWCstring( serverProfile_.serverSettings().authData.Login );
        currentUserName_  =  Utf8ToWCstring( serverProfile_.serverSettings().authData.Login );
    }
    CString folderTitle;
    if ( uploadEngineData->SupportsFolders ) {
         folderTitle = Utf8ToWCstring( serverProfile_.folderTitle() );
        //accountInfoText += CString(_T("\r\n")) + TR("Folder/album:") + _T(" ");
        /*if ( folderTitle.IsEmpty() ) {
            folderTitle = TR("not set");
        } */
        
    }
    SetDlgItemText(IDC_ACCOUNTINFO, accountInfoText);
    bool showAccount = uploadEngineData->NeedAuthorization != 0 && showServerParams;
    GuiTools::ShowDialogItem(m_hWnd, IDC_ACCOUNTINFO, showAccount);
    accountLink_.SetLabel(accountInfoText);
    SetDlgItemText(IDC_FOLDERLABEL, folderTitle);
    
    GuiTools::ShowDialogItem(m_hWnd, IDC_USERICON, showAccount);
    
    bool showFolder = !folderTitle.IsEmpty() && showServerParams;

    RECT rect;
    int settingsBtnPlaceHolderId = ( showFolder || showAccount ) ? IDC_SETTINGSBUTTONPLACEHOLDER : IDC_SETTINGSBUTTONPLACEHOLDER2;
    ::GetWindowRect(GetDlgItem(settingsBtnPlaceHolderId), &rect);
    ::MapWindowPoints(0, m_hWnd, (LPPOINT)&rect, 2);
    settingsButtonToolbar_.SetWindowPos(0, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);
    settingsButtonToolbar_.ShowWindow(showServerParams ? SW_SHOW : SW_HIDE);

    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERLABEL, showFolder );
    GuiTools::ShowDialogItem(m_hWnd, IDC_FOLDERICON, showFolder );
    /*RECT accountLabelRect = */GuiTools::AutoSizeStaticControl(GetDlgItem(IDC_ACCOUNTINFO));
    //int folderIconX = accountLabelRect.right + GuiTools::dlgX(10);
    //::SetWindowPos(GetDlgItem(IDC_FOLDERICON), 0, folderIconX, accountLabelRect.top, 0, 0, SWP_NOSIZE );
    //::SetWindowPos(GetDlgItem(IDC_FOLDERLABEL), 0, folderIconX + 16 + GuiTools::dlgX(3), accountLabelRect.top, 0, 0, SWP_NOSIZE );
}

void CServerSelectorControl::setShowDefaultServerItem(bool show) {
    showDefaultServerItem_ = show;
}

void CServerSelectorControl::setServersMask(int mask) {
    serversMask_ = mask;
}

void CServerSelectorControl::notifyChange()
{
    ::SendMessage(GetParent(), WM_SERVERSELECTCONTROL_CHANGE, (WPARAM)m_hWnd, 0);
    if (OnChange)
    {
        OnChange(this);
    }
}

void CServerSelectorControl::notifyServerListChanged()
{
    ::SendMessage(GetParent(), WM_SERVERSELECTCONTROL_SERVERLIST_CHANGED, (WPARAM)m_hWnd, 0);
}

void CServerSelectorControl::updateServerList()
{
    serverComboBox_.ResetContent();
    comboBoxImageList_.Destroy();
    comboBoxImageList_.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
    

    if ( showDefaultServerItem_ ) {
        serverComboBox_.AddItem(TR("By default"), -1, -1, 0, reinterpret_cast<LPARAM>( strdup("default") ));
    }
    serverComboBox_.AddItem(L"", -1, -1, 0, reinterpret_cast<LPARAM>( strdup("random") ) );

    CIcon hImageIcon = NULL, hFileIcon = NULL;
    int selectedIndex = 0;
    int addedItems = 0;
    std::string selectedServerName = serverProfile_.serverName();
    TCHAR line[40];
    for ( int i=0; i < ARRAY_SIZE(line)-1; i++ ) {
        line[i] = '-';
    }
    line[ARRAY_SIZE(line)-1] = 0;
    for ( int mask = 1; mask <= 4; mask*=2 ) {
        int currentLoopMask = mask & serversMask_;
        if (!currentLoopMask) {
            continue;
        }
        if ( addedItems && currentLoopMask ) {
            serverComboBox_.AddItem(line, -1, -1, 0,  0 );
        }
        for( int i = 0; i < _EngineList->count(); i++) {    
            CUploadEngineData * ue = _EngineList->byIndex( i ); 

            if (serversMask_ != smUrlShorteners && !ue->hasType(CUploadEngineData::TypeFileServer) && !ue->hasType(CUploadEngineData::TypeImageServer)) {
                continue;
            }
            if (!ue->hasType(CUploadEngineData::TypeImageServer) && ((currentLoopMask & smImageServers) == smImageServers)) {
                continue;
            }
            if (!ue->hasType(CUploadEngineData::TypeFileServer) && ((currentLoopMask & smFileServers) == smFileServers)) {
                continue;
            }

            if ( !ue->hasType(CUploadEngineData::TypeUrlShorteningServer) && (currentLoopMask & smUrlShorteners) ) {
                continue;
            }
            HICON hImageIcon = _EngineList->getIconForServer(ue->Name);
            int nImageIndex = -1;
            if ( hImageIcon ) {
                nImageIndex = comboBoxImageList_.AddIcon( hImageIcon);
            }
            char *serverName = new char[ue->Name.length() + 1];
            lstrcpyA( serverName, ue->Name.c_str() );
            int itemIndex = serverComboBox_.AddItem( Utf8ToWCstring( ue->Name ), nImageIndex, nImageIndex, 1, reinterpret_cast<LPARAM>( serverName ) );
            if ( ue->Name == selectedServerName ){
                selectedIndex = itemIndex;
            }
            addedItems++;
        }
    }
    if (serversMask_ != smUrlShorteners ) {
        serverComboBox_.AddItem(line, -1, -1, 0,  0 );
        serverComboBox_.AddItem(  TR("Add FTP server..."), -1, -1, 1, reinterpret_cast<LPARAM>( kAddFtpServer ) );
        serverComboBox_.AddItem(  TR("Add local folder..."), -1, -1, 1, reinterpret_cast<LPARAM>( kAddDirectoryAsServer ) );
    }

    serverComboBox_.SetImageList( comboBoxImageList_ );
    serverComboBox_.SetCurSel( selectedIndex );
    serverChanged();
}

bool CServerSelectorControl::isAccountChosen()
{
    CUploadEngineData* ued = serverProfile_.uploadEngineData();
    return !serverProfile_.profileName().empty() || (ued && ued->NeedAuthorization != CUploadEngineData::naObligatory);
}

LRESULT CServerSelectorControl::OnAccountClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    
    CMenu sub;    
    MENUITEMINFO mi;
    ZeroMemory(&mi,sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_TYPE|MIIM_ID;
    mi.fType = MFT_STRING;
    sub.CreatePopupMenu();
    CUploadEngineData* uploadEngine = serverProfile_.uploadEngineData();



        std::map <std::string, ServerSettingsStruct>& serverUsers = Settings.ServersSettings[serverProfile_.serverName()];

        if(/*uploadEngine->UsingPlugin &&*/ (serverUsers.size()>1 || serverUsers.find("") == serverUsers.end()) )
        {
            bool addedSeparator = false;
            
            int i =0;
            //ShowVar((int)serverUsers.size() );
            if ( serverUsers.size() && !serverProfile_.profileName().empty()) {
                mi.wID = IDC_LOGINMENUITEM;
                mi.dwTypeData  = TR("Change account settings");
                sub.InsertMenuItem(i++, true, &mi);
            } else {
                addedSeparator = true;
            }

            menuOpenedUserNames_.clear();

            int command = IDC_USERNAME_FIRST_ID;
            HICON userIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONUSER));
            
            for( std::map <std::string, ServerSettingsStruct>::iterator it = serverUsers.begin(); it!= serverUsers.end(); ++it ) {
                //    CString login = Utf8ToWCstring(it->second.authData.Login);
                CString login = Utf8ToWCstring(it->first);
                if (!login.IsEmpty() )/*&& it->second.authData.DoAuth**/ {
                    if ( !addedSeparator ) {
                        ZeroMemory(&mi,sizeof(mi));
                        mi.cbSize = sizeof(mi);
                        mi.fMask = MIIM_TYPE|MIIM_ID;
                        mi.wID =  1;
                        mi.fType = MFT_SEPARATOR;

                        sub.InsertMenuItem(i++, true, &mi);
                        addedSeparator =  true;
                    }
                    ZeroMemory(&mi,sizeof(mi));
                    mi.cbSize = sizeof(mi);

                    mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
                    mi.fType = MFT_STRING;
                    mi.wID = command;

                    mi.dwTypeData  = (LPWSTR)(LPCTSTR)login;

                    mi.hbmpItem =  WinUtils::IsVista() ? iconBitmapUtils_->HIconToBitmapPARGB32(userIcon): HBMMENU_CALLBACK;
                    if ( mi.hbmpItem ) {
                        mi.fMask |= MIIM_BITMAP;
                    }
                    menuOpenedUserNames_.push_back(login);
                    sub.InsertMenuItem(i++, true, &mi);
                    command++;
                }

            }
            if ( uploadEngine->NeedAuthorization != CUploadEngineData::naObligatory ) {
                ZeroMemory(&mi,sizeof(mi));
                mi.cbSize = sizeof(mi);
                mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
                mi.fType = MFT_STRING;
                mi.wID = IDC_NO_ACCOUNT;

                mi.dwTypeData  = (LPWSTR)(LPCTSTR)TR("<no authentication>");
                sub.InsertMenuItem(i++, true, &mi);
            }



            ZeroMemory(&mi,sizeof(mi));
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_TYPE|MIIM_ID;
            mi.wID = 1;
            mi.fType = MFT_SEPARATOR;


            sub.InsertMenuItem(i++, true, &mi);


            ZeroMemory(&mi,sizeof(mi));
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
            mi.fType = MFT_STRING;
            mi.wID = IDC_ADD_ACCOUNT ;

            mi.dwTypeData  = (LPWSTR)(LPCTSTR)TR("New account...");


            sub.InsertMenuItem(i++, true, &mi);
            sub.SetMenuDefaultItem(0,TRUE);
        } else {
            addAccount();
            return 0;
        }
        
        RECT rc={0,0,0,0};
        ::GetClientRect(GetDlgItem(IDC_ACCOUNTINFO), &rc);
        ::ClientToScreen(GetDlgItem(IDC_ACCOUNTINFO), (LPPOINT)&rc);
        ::ClientToScreen(GetDlgItem(IDC_ACCOUNTINFO), 1+(LPPOINT)&rc);

    TPMPARAMS excludeArea;
    ZeroMemory(&excludeArea, sizeof(excludeArea));
    excludeArea.cbSize = sizeof(excludeArea);
    excludeArea.rcExclude = rc;
    sub.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
    updateInfoLabel();
    return 0;
}

LRESULT CServerSelectorControl::OnAddAccountClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    addAccount();
    return 0;
}

LRESULT CServerSelectorControl::OnLoginMenuItemClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ServerSettingsStruct  ss = serverProfile_.serverSettings();
    std::string UserName = ss.authData.Login; 
    //bool prevAuthEnabled = ss.authData.DoAuth;
    ServerProfile copy = serverProfile_;
    CLoginDlg dlg(copy, uploadEngineManager_);
    
    if( dlg.DoModal(m_hWnd) == IDOK)
    {
        copy.setProfileName(WCstringToUtf8(dlg.accountName()));
        if(Utf8ToWCstring(UserName) != dlg.accountName())
        {
            
            copy.setFolderId("");
            copy.setFolderTitle("");
            copy.setFolderUrl("");
            serverProfile_ = copy;
            //iuPluginManager.UnloadPlugins();
            //_EngineList->DestroyCachedEngine(WCstringToUtf8(serverProfile_.serverName()), WCstringToUtf8(serverProfile_.profileName()));
        }
        notifyChange();
        updateInfoLabel();
    }
    return 0;
}

LRESULT CServerSelectorControl::OnNoAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    serverProfile_.setProfileName("");
    serverProfile_.setFolderId("");
    serverProfile_.setFolderTitle("");
    serverProfile_.setFolderUrl("");
    updateInfoLabel();
    notifyChange();
    return 0;
}

LRESULT CServerSelectorControl::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return MA_NOACTIVATE;
}

void CServerSelectorControl::createSettingsButton() {
    CIcon ico = (HICON)LoadImage(GetModuleHandle(0),  MAKEINTRESOURCE(IDI_ICONSETTINGS2), IMAGE_ICON    , 16,16,0);
    RECT profileRect;
    ::GetWindowRect(GetDlgItem(IDC_SETTINGSBUTTONPLACEHOLDER), &profileRect);
    ::MapWindowPoints(0, m_hWnd, (LPPOINT)&profileRect, 2);

    settingsButtonToolbar_.Create(m_hWnd,profileRect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD | TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|CCS_RIGHT|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/TBSTYLE_TOOLTIPS|CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
    settingsButtonToolbar_.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
    settingsButtonToolbar_.SetButtonStructSize();
    settingsButtonToolbar_.SetButtonSize(17,17);

    CImageList list;
    list.Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
    list.AddIcon(ico);
    settingsButtonToolbar_.SetImageList(list);
    settingsButtonToolbar_.AddButton(IDC_EDIT, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0,TR("Server and authentication settings"), 0);
}

void CServerSelectorControl::setShowImageProcessingParams(bool show) {
    showImageProcessingParams_ = show;
}

void CServerSelectorControl::setShowParamsLink(bool show) {
    showParamsLink_ = show;
}

LRESULT CServerSelectorControl::OnImageProcessingParamsClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    int serverComboElementIndex = serverComboBox_.GetCurSel();
    std::string serverName = reinterpret_cast<char*>( serverComboBox_.GetItemData(serverComboElementIndex) );
    CUploadParamsDlg dlg(serverProfile_, showImageProcessingParams_, defaultServer_);
    if (dlg.DoModal(isChildWindow_ ? m_hWnd : GetParent()) == IDOK) {
        serverProfile_.setImageUploadParams(dlg.imageUploadParams());
    }
    return 0;
}

LRESULT CServerSelectorControl::OnUserNameMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int userNameIndex = wID - IDC_USERNAME_FIRST_ID;
    CString userName = menuOpenedUserNames_[userNameIndex];
    serverProfile_.setProfileName(WCstringToUtf8(userName));
    ServerSettingsStruct& sss = serverProfile_.serverSettings();

    serverProfile_.setFolderId(sss.defaultFolder.getId());
    serverProfile_.setFolderTitle(sss.defaultFolder.getTitle());
    serverProfile_.setFolderUrl(sss.defaultFolder.viewUrl);

    notifyChange();
    updateInfoLabel();
    return 0;
}

int CServerSelectorControl::showPopup(HWND parent, POINT pt)
{
    // Code from \Program Files\Microsoft SDKs\Windows\v7.1\Samples\winui\shell\legacysamples\fakemenu\fakemenu.cpp
    isChildWindow_ = false;
    if (Create(parent) == NULL) {
        ATLTRACE(_T("CServerSelectorControl dialog creation failed!  :( sorry\n"));
        return 0;
    }
    int nRet(-1);
    SetWindowPos(0, pt.x, pt.y, 0, 0, SWP_NOACTIVATE | SWP_NOZORDER|SWP_NOSIZE);
    ShowWindow(SW_SHOWNOACTIVATE);

    BOOL bMenuDestroyed(FALSE);
    HWND hwndOwner = GetWindow(GW_OWNER);
    HWND hwndPopup = m_hWnd;
    // We want to receive all mouse messages, but since only the active
    // window can capture the mouse, we have to set the capture to our
    // owner window, and then steal the mouse messages out from under it.
    //::SetCapture(hwndOwner);

    // Go into a message loop that filters all the messages it receives
    // and route the interesting ones to the color picker window.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // If our owner stopped being the active window (e.g. the user
        // Alt+Tab'd to another window in the meantime), then stop.
        HWND hwndActive = GetActiveWindow();
        if (hwndActive != hwndPopup && hwndActive != hwndOwner && !::IsChild(hwndActive, hwndOwner) /*||
            GetCapture() != hwndOwner*/)
        {
            break;
        }

        
        bool isChildMessage = ::IsChild(hwndPopup, msg.hwnd)!=FALSE;
        if (!isChildMessage)
        {
            TCHAR className[MAX_PATH];
            if (::GetClassName(msg.hwnd, className, MAX_PATH) != 0)
            {
                isChildMessage = lstrcmp(className, _T("ComboLBox"))==0; // Style of ComboboxEx popup list box
            }
        }
        bool breakLoop = false;
        // At this point, we get to snoop at all input messages before
        // they get dispatched.  This allows us to route all input to our
        // popup window even if really belongs to somebody else.

        // All mouse messages are remunged and directed at our popup
        // menu. If the mouse message arrives as client coordinates, then
        // we have to convert it from the client coordinates of the original
        // target to the client coordinates of the new target.
        switch (msg.message)
        {
            // These mouse messages arrive in client coordinates, so in
            // addition to stealing the message, we also need to convert the
            // coordinates.
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            if (msg.hwnd == hwndOwner || ::IsChild(hwndOwner, msg.hwnd))
            {
                breakLoop = true;
                break;
            }
        case WM_MOUSEMOVE:
            if (!isChildMessage) {
                pt.x = (short)LOWORD(msg.lParam);
                pt.y = (short)HIWORD(msg.lParam);
                ::MapWindowPoints(msg.hwnd, hwndPopup, &pt, 1);
                msg.lParam = MAKELPARAM(pt.x, pt.y);
                msg.hwnd = hwndPopup;
            }
            break;

            // These mouse messages arrive in screen coordinates, so we just
            // need to steal the message.
        
        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONUP:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONUP:
        case WM_NCMBUTTONDBLCLK:
        case WM_SETCURSOR:
            if (msg.hwnd == hwndOwner || ::IsChild(hwndOwner, msg.hwnd))
            {
                breakLoop = true;
                break;
            }
        case WM_NCMOUSEMOVE:
            if (!isChildMessage) {
                msg.hwnd = hwndPopup;
            }
            break;

            // We need to steal all keyboard messages, too.
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
        case WM_DEADCHAR:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
        case WM_SYSDEADCHAR:
            if (!isChildMessage)
            {
                msg.hwnd = hwndPopup;
            }
            if (msg.wParam == VK_ESCAPE && (msg.hwnd == hwndPopup || ::IsChild(hwndPopup, msg.hwnd)))
            {
                breakLoop = true;
                break;
            }
            break;
        }
        if (breakLoop)
        {
            break;
        }
        if (!::IsDialogMessage(hwndPopup, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    
        
        // If our owner stopped being the active window (e.g. the user
        // Alt+Tab'd to another window in the meantime), then stop.
        hwndActive = GetActiveWindow();
        if (hwndActive != hwndPopup && hwndActive != hwndOwner && !::IsChild(hwndActive, hwndOwner)/* ||
            GetCapture() != hwndOwner*/)
        {
            break;
        }
    }

    // Clean up the capture we created.
    //ReleaseCapture();

    isPopingUp_ = false;
    // If we got a WM_QUIT message, then re-post it so the caller's message
    // loop will see it.
    if (msg.message == WM_QUIT)
    {
        PostQuitMessage((int)msg.wParam);
    }


    if (!bMenuDestroyed) DestroyWindow();

    return nRet;
}

bool CServerSelectorControl::exitPopup(int nCommandId)
{
    BOOL bRet = SetProp(m_hWnd, MENU_EXIT_NOTIFY, (HANDLE)1);
    SetProp(m_hWnd, MENU_EXIT_COMMAND_ID, (HANDLE)nCommandId);
    return bRet !=FALSE;
}

DLGTEMPLATE* CServerSelectorControl::GetTemplate()
{
    HINSTANCE hInst = GetModuleHandle(0);
    HRSRC res = FindResource(hInst, MAKEINTRESOURCE(IDD), RT_DIALOG);
    DLGTEMPLATE* dit = (DLGTEMPLATE*)LockResource(LoadResource(hInst, res));

    unsigned long sizeDlg = ::SizeofResource(hInst, res);
    HGLOBAL hMyDlgTemplate = ::GlobalAlloc(GPTR, sizeDlg);
    DLGTEMPLATEEX *pMyDlgTemplate = (DLGTEMPLATEEX *)::GlobalLock(hMyDlgTemplate);
    ::memcpy(pMyDlgTemplate, dit, sizeDlg);

    if (isChildWindow_)
    {
        //pMyDlgTemplate->style = pMyDlgTemplate->style & ~ WS_POPUP;
        pMyDlgTemplate->style = pMyDlgTemplate->style | WS_CHILD;
    }
    else
    {
        pMyDlgTemplate->style -= WS_CHILD;
        pMyDlgTemplate->style -= DS_CONTROL;
        pMyDlgTemplate->exStyle |= WS_EX_NOACTIVATE | 
            WS_EX_TOOLWINDOW |      // So it doesn't show up in taskbar
            WS_EX_DLGMODALFRAME |   // Get the edges right
            WS_EX_WINDOWEDGE /*|
            WS_EX_TOPMOST*/; // with this style window is overlapping modal dialogs (server params dialog, add ftp server, etc...)

        pMyDlgTemplate->style = pMyDlgTemplate->style | WS_POPUP | WS_BORDER/* | WS_CAPTION*/ ;
    }
    return (DLGTEMPLATE*)pMyDlgTemplate;
}