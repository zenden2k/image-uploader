/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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
#include "UploadSettings.h"

#include <boost/format.hpp>

#include "atlheaders.h"
#include "ServerFolderSelect.h"
#include "NewFolderDlg.h"
#include "ServerParamsDlg.h"
#include "Gui/GuiTools.h"
#include "Func/MyEngineList.h"
#include "Gui/Dialogs/SettingsDlg.h"
#include "Gui/IconBitmapUtils.h"
#include "Func/WinUtils.h"
#include "AddFtpServerDialog.h"
#include "AddDirectoryServerDialog.h"
#include "Gui/Controls/ServerSelectorControl.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "LoginDlg.h"
#include "ServerProfileGroupSelectDialog.h"
#include "Core/ServiceLocator.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Core/WinServerIconCache.h"
#include "Func/IuCommonFunctions.h"
#include "StatusDlg.h"
#include "Core/FileTypeCheckTask.h"
#include "Gui/Dialogs/FileFormatCheckErrorDlg.h"

namespace {
    struct ResizePreset {
        std::wstring width;
        std::wstring height;
        std::wstring title;
        int menuGroup;
    };

    // Maximum number of presets is limited to 100 items 
    // (IDC_RESIZEPRESETMENU_LAST_ID - IDC_RESIZEPRESETMENU_FIRST_ID)
    ResizePreset resizePresets[] = {
        {L"",     L"",     L"", 0}, // No resize at all
        {L"800",  L"600",  L"", 1}, // If title is empty, it is being formatted like "WxH"
        {L"1024", L"768",  L"", 1},
        {L"1600", L"1200", L"", 1},
        {L"1280",  L"720",  L"", 2},
        {L"1600", L"900",  L"", 2},
        {L"1920", L"1080", L"", 2},
        {L"25%",  L"25%",  L"25%", 3},
        {L"50%",  L"50%",  L"50%", 3},
        {L"75%",  L"75%",  L"75%", 3}
    };
}

CUploadSettings::CUploadSettings(CMyEngineList* EngineList, UploadEngineManager* uploadEngineManager, WinServerIconCache* iconCache) 
    :convert_profiles_(ServiceLocator::instance()->settings<WtlGuiSettings>()->ConvertProfiles),
    iconCache_(iconCache)
{
    auto* settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    nImageIndex = nFileIndex = -1;
    m_EngineList = EngineList;
    m_ProfileChanged  = false;
    m_CatchChanges = false;
    iconBitmapUtils_ = std::make_unique<IconBitmapUtils>();
    useServerThumbnailsTooltip_ = nullptr;
    uploadEngineManager_ = uploadEngineManager;
    using namespace std::placeholders;
    settingsChangedConnection_ = settings->onChange.connect(std::bind(&CUploadSettings::settingsChanged, this, _1));
}

CUploadSettings::~CUploadSettings() {
    try {
        settingsChangedConnection_.disconnect();
    } catch ( const std::exception& ex) {
        LOG(ERROR) << ex.what();
    }
}

void CUploadSettings::settingsChanged(BasicSettings* settingsBase)
{
    auto* settings = dynamic_cast<CommonGuiSettings*>(settingsBase);
    if (settings) {
        if (!settings->imageServer.isEmpty()) {
            const std::string templateName = settings->imageServer.getByIndex(0).getImageUploadParamsRef().getThumbRef().TemplateName;
            if (sessionImageServer_.isEmpty()) {
                sessionImageServer_.getByIndex(0).getImageUploadParamsRef().getThumbRef().TemplateName = templateName;
            }

        }
    }
}

void CUploadSettings::TranslateUI()
{
    TRC(IDC_FORMATLABEL, "Format:");
    TRC(IDC_QUALITYLABEL, "Quality:");
    TRC(IDC_RESIZEBYWIDTH, "Change width:");
    TRC(IDC_XLABEL, "and/or height:");
    TRC(IDC_PROFILELABEL, "Profile:");
    TRC(IDC_IMAGEPARAMETERS, "Image settings");
    TRC(IDC_KEEPASIS, "Process images before upload");
    TRC(IDC_THUMBSETTINGS, "Thumbnails");
    TRC(IDC_CREATETHUMBNAILS, "Create thumbnails");
    TRC(IDC_IMAGESERVERGROUPBOX, "Server to host your images");
    TRC(IDC_USESERVERTHUMBNAILS, "Use server-side thumbnails");
    TRC(IDC_WIDTHLABEL, "Thumbnail's size:");
    TRC(IDC_ADDFILESIZE, "Add text on thumbnail");
    TRC(IDC_PRESSUPLOADBUTTON, "Click button \"Upload\" for starting process of uploading.");
    TRC(IDC_FILESERVERGROUPBOX, "Server to host other file formats");
    TRC(IDC_SKIPANIMATEDCHECKBOX, "Skip animated");
    useServerThumbnailsTooltip_ = GuiTools::CreateToolTipForWindow(GetDlgItem(IDC_USESERVERTHUMBNAILS), TR("This means that the thumbnail will be created by site, not the program."));
}

LRESULT CUploadSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PageWnd = m_hWnd;
    sessionImageServer_ = WizardDlg->getSessionImageServer();
    sessionFileServer_ = WizardDlg->getSessionFileServer();

    moreImageServersLink_.SubclassWindow(GetDlgItem(IDC_CHOOSEMOREIMAGESERVERSLABEL));
    moreImageServersLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON;
    moreImageServersLink_.SetLabel(TR("Choose more servers..."));
    moreImageServersLink_.SetToolTipText(TR("Choose more servers..."));
    moreImageServersLink_.m_clrLink = GetSysColor(COLOR_WINDOWTEXT);

    moreFileServersLink_.SubclassWindow(GetDlgItem(IDC_CHOOSEMOREFILESERVERS));
    moreFileServersLink_.m_dwExtendedStyle |= HLINK_UNDERLINEHOVER | HLINK_COMMANDBUTTON;
    moreFileServersLink_.SetLabel(TR("Choose more servers..."));
    moreFileServersLink_.SetToolTipText(TR("Choose more servers..."));
    moreFileServersLink_.m_clrLink = GetSysColor(COLOR_WINDOWTEXT);

    //m_ThumbSizeEdit.SubclassWindow(GetDlgItem(IDC_QUALITYEDIT));
    TranslateUI();

    int iconWidth = ::GetSystemMetrics(SM_CXSMICON);
    int iconHeight = ::GetSystemMetrics(SM_CYSMICON);

    //CClientDC dc(HWND_DESKTOP);
    // Get color depth (minimum requirement is 32-bits for alpha blended images).
    //int iBitsPixel = GetDeviceCaps(dc,BITSPIXEL);
    /*if (iBitsPixel >= 32)
    {
        hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP5));
        m_PlaceSelectorImageList.Create(16,16,ILC_COLOR32,0,6);
        m_PlaceSelectorImageList.Add(hBitmap, (HBITMAP) NULL);
    }
    else*/ {
        //hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_SERVERTOOLBARBMP2));
        m_PlaceSelectorImageList.Create(iconWidth, iconHeight, ILC_COLOR32, 0, 6);
        CIcon iconUser;
        iconUser.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICONUSER), iconWidth, iconHeight);
        m_PlaceSelectorImageList.AddIcon(iconUser);
        CIcon iconFolder;
        iconFolder.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICONFOLDER2), iconWidth, iconHeight);
        m_PlaceSelectorImageList.AddIcon(iconFolder);

        CIcon iconSeparator;
        iconSeparator.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICONSEPARATOR), iconWidth, iconHeight);
        m_PlaceSelectorImageList.AddIcon(iconSeparator);

        //m_PlaceSelectorImageList.Add(hBitmap,RGB(255,0,255));
    }


    HICON ico = {};
    
    LoadIconWithScaleDown(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_DROPDOWN), iconWidth, iconHeight, &ico);
    iconDropdown_ = ico;
    m_ResizePresetIconButton.m_hWnd = GetDlgItem(IDC_RESIZEPRESETSBUTTON);
    m_ResizePresetIconButton.SetIcon(iconDropdown_);

    m_ShorteningServerButton.m_hWnd = GetDlgItem(IDC_SHORTENINGURLSERVERBUTTON);
    m_ShorteningServerButton.SetIcon(iconDropdown_);

    iconEdit_.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICONEDIT), iconWidth, iconHeight);

    RECT profileRect;
    ::GetWindowRect(GetDlgItem(IDC_EDITPROFILE), &profileRect);
    ::MapWindowPoints(0, m_hWnd, reinterpret_cast<LPPOINT>(&profileRect), 2);

    m_ProfileEditToolbar.Create(m_hWnd, profileRect,_T(""), WS_CHILD | WS_VISIBLE | WS_CHILD | TBSTYLE_LIST | TBSTYLE_FLAT | CCS_NORESIZE | /*CCS_BOTTOM |CCS_ADJUSTABLE|*/TBSTYLE_TOOLTIPS | CCS_NODIVIDER | TBSTYLE_AUTOSIZE);
    m_ProfileEditToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
    m_ProfileEditToolbar.SetButtonStructSize();
    m_ProfileEditToolbar.SetButtonSize(iconWidth + 2, iconHeight + 2);

    m_profileEditToolbarImageList.Create(iconWidth, iconHeight, ILC_COLOR32 | ILC_MASK, 0, 6);
    m_profileEditToolbarImageList.AddIcon(iconEdit_);
    m_ProfileEditToolbar.SetImageList(m_profileEditToolbarImageList);
    m_ProfileEditToolbar.AddButton(IDC_EDITPROFILE, TBSTYLE_BUTTON | BTNS_AUTOSIZE, TBSTATE_ENABLED, 0,TR("Edit Profile"), 0);

    RECT Toolbar1Rect;
    ::GetWindowRect(GetDlgItem(IDC_IMAGESERVERGROUPBOX), &Toolbar1Rect);
  
    ::MapWindowPoints(0, m_hWnd, reinterpret_cast<LPPOINT>(&Toolbar1Rect), 2);
    Toolbar1Rect.top += GuiTools::dlgY(9);
    Toolbar1Rect.bottom -= GuiTools::dlgY(3);
    Toolbar1Rect.left += GuiTools::dlgX(6);
    Toolbar1Rect.right -= GuiTools::dlgX(6);

    RECT Toolbar2Rect;
    ::GetWindowRect(GetDlgItem(IDC_FILESERVERGROUPBOX), &Toolbar2Rect);
    ::MapWindowPoints(0, m_hWnd, reinterpret_cast<LPPOINT>(&Toolbar2Rect), 2);
    Toolbar2Rect.top += GuiTools::dlgY(9);
    Toolbar2Rect.bottom -= GuiTools::dlgY(3);
    Toolbar2Rect.left += GuiTools::dlgX(6);
    Toolbar2Rect.right -= GuiTools::dlgX(6);

    for(int i = 0; i<2; i++)
    {
        CToolBarCtrl& CurrentToolbar = (i == 0) ? Toolbar: FileServerSelectBar;
        CurrentToolbar.Create(m_hWnd,i?Toolbar2Rect:Toolbar1Rect,_T(""), WS_CHILD|WS_VISIBLE|WS_CHILD |WS_TABSTOP| TBSTYLE_LIST |TBSTYLE_FLAT| CCS_NORESIZE|/*CCS_BOTTOM |CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );
        
        CurrentToolbar.SetButtonStructSize();
        CurrentToolbar.SetButtonSize(30,18);
        CurrentToolbar.SetImageList(m_PlaceSelectorImageList);
        
        CurrentToolbar.AddButton(IDC_SERVERBUTTON, TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, -1, TR("Choose server..."), 0);
        CurrentToolbar.AddButton(IDC_TOOLBARSEPARATOR1, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, _T(""), 0);
        
        CurrentToolbar.AddButton(IDC_LOGINTOOLBUTTON + !i, /*TBSTYLE_BUTTON*/TBSTYLE_DROPDOWN |BTNS_AUTOSIZE, TBSTATE_ENABLED, 0, _T(""), 0);
        CurrentToolbar.AddButton(IDC_TOOLBARSEPARATOR2, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, _T(""), 0);
        
        CurrentToolbar.AddButton(IDC_SELECTFOLDER, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Choose folder..."), 0);
        CurrentToolbar.AutoSize();
        CurrentToolbar.SetWindowPos(i == 0 ? GetDlgItem(IDC_IMAGESERVERGROUPBOX) : Toolbar.m_hWnd, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
        SIZE toolbarSize;
        if (CurrentToolbar.GetMaxSize(&toolbarSize)) {
            RECT rc = i ? Toolbar2Rect : Toolbar1Rect;
            rc.top = rc.top + (rc.bottom - rc.top - toolbarSize.cy) / 2;
            CurrentToolbar.SetWindowPos(nullptr, &rc, SWP_NOZORDER);
        }

    }

    Toolbar.SetWindowLong(GWL_ID, IDC_IMAGETOOLBAR);
    FileServerSelectBar.SetWindowLong(GWL_ID, IDC_FILETOOLBAR);
    
    SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
    SendDlgItemMessage(IDC_THUMBQUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1));
    
    GuiTools::AddComboBoxItems(m_hWnd, IDC_FORMATLIST, 6, TR("Auto"), _T("JPEG"), _T("PNG"), _T("GIF"), _T("WebP"), _T("WebP (lossless)"));
    
    ShowParams();
    CString profileName = sessionImageServer_.isEmpty() ? U2W(sessionImageServer_.getByIndex(0).getImageUploadParams().ImageProfileName) : _T("");
    if (convert_profiles_.find(profileName) == convert_profiles_.end()) {
        profileName = _T("Default");
    }

    ShowParams(profileName);
    UpdateProfileList();
    UpdateAllPlaceSelectors();
    return 1;  // Let the system set the focus
}

LRESULT CUploadSettings::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CUploadSettings::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CUploadSettings::OnBnClickedKeepasis(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    bool checked = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0, 0)!=FALSE;
    GuiTools::EnableNextN(GetDlgItem(IDC_KEEPASIS), 15, checked);
    m_ProfileEditToolbar.EnableWindow(checked);
    return 0;
}

void CUploadSettings::ShowParams(/*UPLOADPARAMS params*/)
{
    ImageUploadParams params = sessionImageServer_.isEmpty() ? ImageUploadParams() : sessionImageServer_.getByIndex(0).getImageUploadParams();
    SendDlgItemMessage(IDC_KEEPASIS, BM_SETCHECK, params.ProcessImages);
    SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_SETCURSEL, static_cast<int>(params.getThumb().Format));
    SendDlgItemMessage(IDC_CREATETHUMBNAILS, BM_SETCHECK, params.CreateThumbs);
    SendDlgItemMessage(IDC_ADDFILESIZE, BM_SETCHECK, params.getThumb().AddImageSize);
    SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_SETCHECK, params.UseServerThumbs);

    bool shortenImages = getSessionImageServerItem().shortenLinks();
    bool shortenFiles = getSessionFileServerItem().shortenLinks();
    int shortenLinks = BST_INDETERMINATE;
    int checkboxStyle = BS_AUTO3STATE;
    HWND checkbox = GetDlgItem(IDC_SHORTENLINKSCHECKBOX);
    if (shortenImages == shortenFiles)
    {
        shortenLinks = shortenImages ? BST_CHECKED : BST_UNCHECKED;
        checkboxStyle = BS_AUTOCHECKBOX;
    } 
    ::SetWindowLong(checkbox, GWL_STYLE, (::GetWindowLong(checkbox, GWL_STYLE) & ~(BS_AUTO3STATE | BS_AUTOCHECKBOX)) | checkboxStyle);
    SendDlgItemMessage(IDC_SHORTENLINKSCHECKBOX, BM_SETCHECK, shortenLinks);
    updateUrlShorteningCheckboxLabel();
}

LRESULT CUploadSettings::OnBnClickedCreatethumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
    BOOL checked = SendDlgItemMessage(IDC_CREATETHUMBNAILS, BM_GETCHECK, 0, 0);
    
    ::EnableWindow(GetDlgItem(IDC_THUMBWIDTH), checked);
    ::EnableWindow(GetDlgItem(IDC_THUMBHEIGHT), checked);
    ::EnableWindow(GetDlgItem(IDC_ADDFILESIZE), checked);
    ::EnableWindow(GetDlgItem(IDC_WIDTHLABEL), checked);
    ::EnableWindow(GetDlgItem(IDC_USESERVERTHUMBNAILS), checked);
    ::EnableWindow(GetDlgItem(IDC_PXLABEL), checked);
        
    return 0;
}

bool CUploadSettings::OnNext()
{    
    auto *settings = ServiceLocator::instance()->settings<WtlGuiSettings>();
    auto& sessionImageServer = getSessionImageServerItem();
    if (sessionImageServer.isNull()) {
        GuiTools::LocalizedMessageBox(m_hWnd, TR("You have not selected an image server"), TR("Error"), MB_ICONERROR);
        return false;
    }

    if(!sessionImageServer.serverName().empty())
    {
        CUploadEngineData *ue = sessionImageServer.uploadEngineData();
        if (ue->NeedAuthorization == CUploadEngineData::naObligatory && sessionImageServer.profileName().empty())
        { 
            CString errorMsg;
            errorMsg.Format(TR("Upload to server '%s' is impossible without account.\nYou should sign Up on this server and specify your account details in program settings."), (LPCTSTR)Utf8ToWCstring(ue->Name));
            GuiTools::LocalizedMessageBox(m_hWnd, errorMsg, APPNAME, MB_ICONERROR);
            return false;
        }
    }

    auto& sessionFileServer = getSessionFileServerItem();

    if (sessionFileServer.isNull()) {
        GuiTools::LocalizedMessageBox(m_hWnd, TR("You have not selected an file server"), TR("Error"), MB_ICONERROR);
        return false;
    }
    if(!sessionFileServer.serverName().empty())
    {
        CUploadEngineData *ue2 = sessionFileServer.uploadEngineData();
        if (ue2->NeedAuthorization == CUploadEngineData::naObligatory && sessionFileServer.profileName().empty())
        {
            CString errorMsg;
            errorMsg.Format(TR("Please specify authentication settings for '%s' server!"), static_cast<LPCTSTR>(U2W(ue2->Name)));
            GuiTools::LocalizedMessageBox(m_hWnd, errorMsg, APPNAME, MB_ICONWARNING);
            return false;
        }
    }

    ImageUploadParams& imageUploadParams = sessionImageServer.getImageUploadParamsRef();
    imageUploadParams.ProcessImages = SendDlgItemMessage(IDC_KEEPASIS, BM_GETCHECK, 0) == BST_CHECKED;
    imageUploadParams.CreateThumbs = GuiTools::IsChecked(m_hWnd, IDC_CREATETHUMBNAILS);
    imageUploadParams.UseServerThumbs = GuiTools::IsChecked(m_hWnd, IDC_USESERVERTHUMBNAILS);
    imageUploadParams.getThumbRef().AddImageSize = GuiTools::IsChecked(m_hWnd, IDC_ADDFILESIZE);

    int thumbWidth = GetDlgItemInt(IDC_THUMBWIDTH);
    int thumbHeight = GetDlgItemInt(IDC_THUMBHEIGHT);

    if (!thumbWidth && !thumbHeight) {
        thumbWidth = ThumbCreatingParams::DEFAULT_THUMB_WIDTH;
    }

    auto& thumb = imageUploadParams.getThumbRef();
    thumb.Width = thumbWidth;
    thumb.Height = thumbHeight;

    if (thumb.Width && thumb.Height) {
        thumb.ResizeMode = ThumbCreatingParams::trByBoth;
    } else if (thumb.Height) {
        thumb.ResizeMode = ThumbCreatingParams::trByHeight;
    } else {
        thumb.ResizeMode = ThumbCreatingParams::trByWidth;
    }

    

    int shortenLinks = SendDlgItemMessage(IDC_SHORTENLINKSCHECKBOX, BM_GETCHECK);
    if (shortenLinks != BST_INDETERMINATE)
    {
        bool shorten = shortenLinks == BST_CHECKED;
        sessionImageServer.setShortenLinks(shorten);
        sessionFileServer.setShortenLinks(shorten);
    }

    if (settings->CheckFileTypesBeforeUpload && !WizardDlg->checkFileFormats(sessionImageServer_, sessionFileServer_)) {
        return false;
    }

    WizardDlg->setSessionImageServer(sessionImageServer_);
    WizardDlg->setSessionFileServer(sessionFileServer_);
    if ( settings->RememberImageServer ) {
        settings->imageServer = sessionImageServer_;
    }
    if ( settings->RememberFileServer ) {
        settings->fileServer = sessionFileServer_;
    }

    SaveCurrentProfile();

    return true;
}

bool CUploadSettings::OnShow()
{
    ShowPrev();
    ShowNext();
    BOOL temp;

    if (WizardDlg->serversChanged()) {
        sessionImageServer_ = WizardDlg->getSessionImageServer();
        sessionFileServer_ = WizardDlg->getSessionFileServer();
        WizardDlg->setServersChanged(false);
        ShowParams();
    }

    auto* mainDlg = WizardDlg->getPage<CMainDlg>(CWizardDlg::wpMainPage);

    // Reset skip flag, which can be set when checkFileFormats on file list is called
    // CAtlArray doesn't have begin() and end() methods.
    for (size_t i = 0; i < mainDlg->FileList.GetCount(); i++) {
        mainDlg->FileList[i].setSkipped(false);
    }

    CString profileName = sessionImageServer_.isEmpty() ? _T(""): U2W(sessionImageServer_.getByIndex(0).getImageUploadParamsRef().ImageProfileName);
    if (convert_profiles_.find(profileName) == convert_profiles_.end()) {
        profileName = _T("Default");
    }
    ShowParams(profileName);
    UpdateProfileList();
    UpdateAllPlaceSelectors();
    OnBnClickedCreatethumbnails(0, 0, 0, temp);
    OnBnClickedKeepasis(0, 0, 0, temp);
    SetNextCaption(TR("&Upload"));
    EnableNext();
    return true;
}

LRESULT CUploadSettings::OnBnClickedLogin(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
{
    bool ImageServer = (wID % 2)!=0;

    ServerProfile & serverProfile = ImageServer? getSessionImageServerItem() : getSessionFileServerItem();
    CLoginDlg dlg(serverProfile, uploadEngineManager_);

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = Settings->getServerSettings(ImageServer ? getSessionImageServerItem() : getSessionFileServerItem());
 
    std::string UserName = serverSettings ? serverSettings->authData.Login : std::string();
    bool prevAuthEnabled = serverSettings ? serverSettings->authData.DoAuth : false;
    UINT dlgResult = dlg.DoModal(m_hWnd);
        
    if (dlgResult  == IDOK) {
        ServerSettingsStruct* ss = Settings->getServerSettings(ImageServer ? getSessionImageServerItem() : getSessionFileServerItem());
        std::string newUserName = ss ? ss->authData.Login : std::string();
        bool newAuth = ss ? ss->authData.DoAuth : false;
        serverProfile.setProfileName(WCstringToUtf8(dlg.accountName()));
        if(UserName != newUserName || newAuth != prevAuthEnabled) {
            serverProfile.setFolderId("");
            serverProfile.setFolderTitle("");
            serverProfile.setFolderUrl("");
            serverProfile.setParentIds({});
        }
            
        UpdateAllPlaceSelectors();
    } else if (dlgResult == CLoginDlg::ID_DELETEACCOUNT) {
        serverProfile.setFolderId("");
        serverProfile.setFolderTitle("");
        serverProfile.setFolderUrl("");
        serverProfile.setProfileName("");
        serverProfile.setParentIds({});
        UpdateAllPlaceSelectors();
    }
    return 0;
}
    
LRESULT CUploadSettings::OnBnClickedUseServerThumbnails(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    //BOOL checked=SendDlgItemMessage(IDC_USESERVERTHUMBNAILS, BM_GETCHECK, 0, 0);
    return 0;
}
    
LRESULT CUploadSettings::OnBnClickedSelectFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl, BOOL& /*bHandled*/)
{
	bool ImageServer = (hWndCtl == Toolbar.m_hWnd);    
    
    ServerProfile& serverProfile = ImageServer ? getSessionImageServerItem() : getSessionFileServerItem();
    CUploadEngineData *ue = serverProfile.uploadEngineData();

	if (!ue) {
		LOG(ERROR) << "serverProfile.uploadEngineData() cannot be NULL";
		return 0;
	}

    if(ue->SupportsFolders){
        CServerFolderSelect as(serverProfile, uploadEngineManager_);

        as.m_SelectedFolder.id = serverProfile.folderId();
        as.m_SelectedFolder.parentIds = serverProfile.parentIds();
        
        if(as.DoModal(m_hWnd) == IDOK){
            BasicSettings* settings = ServiceLocator::instance()->basicSettings();
            ServerSettingsStruct* serverSettings = settings->getServerSettings(serverProfile);

            if(!as.m_SelectedFolder.id.empty()){
                if (serverSettings) {
                    serverSettings->defaultFolder = as.m_SelectedFolder;
                }
                serverProfile.setFolderId(as.m_SelectedFolder.id);
                serverProfile.setFolderTitle(as.m_SelectedFolder.title);
                serverProfile.setFolderUrl(as.m_SelectedFolder.viewUrl);
                serverProfile.setParentIds(as.m_SelectedFolder.parentIds);
            }
            else {
                if (serverSettings) {
                    serverSettings->defaultFolder = CFolderItem();
                }
                serverProfile.setFolderId("");
                serverProfile.setFolderTitle("");
                serverProfile.setFolderUrl("");
                serverProfile.setParentIds({});
            }
            UpdateAllPlaceSelectors();
        }
    }
    return 0;
}
    
void CUploadSettings::UpdateToolbarIcons()
{
    HICON hImageIcon = NULL, hFileIcon = NULL;

    if (!getSessionImageServerItem().isNull()) {
        hImageIcon = iconCache_->getIconForServer(getSessionImageServerItem().serverName());
    }
        
    if (!getSessionFileServerItem().isNull()) {
        hFileIcon = iconCache_->getIconForServer(getSessionFileServerItem().serverName());
    }
    
    if(hImageIcon)
    {
        if(nImageIndex == -1)
        {
            nImageIndex = m_PlaceSelectorImageList.AddIcon( hImageIcon);
        }
        else nImageIndex= m_PlaceSelectorImageList.ReplaceIcon(nImageIndex, hImageIcon);
    } else nImageIndex =-1;

    if(hFileIcon)
    {
        if(nFileIndex == -1)
        {
            nFileIndex = m_PlaceSelectorImageList.AddIcon( hFileIcon);
        }
        else 
            nFileIndex = m_PlaceSelectorImageList.ReplaceIcon(nFileIndex, hFileIcon);
    } else nFileIndex = -1;

    Toolbar.ChangeBitmap(IDC_SERVERBUTTON, nImageIndex);
    FileServerSelectBar.ChangeBitmap(IDC_SERVERBUTTON, nFileIndex);
}

void CUploadSettings::UpdatePlaceSelector(bool ImageServer)
{
    TBBUTTONINFO bi;
    CToolBarCtrl& CurrentToolbar = ImageServer ? Toolbar: FileServerSelectBar;

//    int nServerIndex = ImageServer? m_nImageServer: m_nFileServer;
    ServerProfile& serverProfile = ImageServer ? getSessionImageServerItem() : getSessionFileServerItem();

    CString serverTitle = (!serverProfile.isNull()) ? Utf8ToWCstring(serverProfile.serverName()) : TR("Choose server");

    ZeroMemory(&bi, sizeof(bi));
    bi.cbSize = sizeof(bi);
    bi.dwMask = TBIF_TEXT;
    bi.pszText = const_cast<LPWSTR>(serverTitle.GetString());
    CurrentToolbar.SetButtonInfo(IDC_SERVERBUTTON, &bi);

    if(serverProfile.isNull())
    { 
        CurrentToolbar.HideButton(IDC_LOGINTOOLBUTTON + ImageServer ,true);
        CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR1, true);
        CurrentToolbar.HideButton(IDC_SELECTFOLDER, true);
        CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR2, true);
        return;
    }

    CUploadEngineData * uploadEngine = ServiceLocator::instance()->engineList()->byName(serverProfile.serverName());
	if (!uploadEngine) {
		LOG(ERROR) << "uploadEngine cannot be NULL";
		return;
	}

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* res = Settings->getServerSettings(serverProfile);
    ServerSettingsStruct serverSettings;
    if (res) {
        serverSettings = *res;
    }
    LoginInfo& li = serverSettings.authData;
    CString login = WinUtils::TrimString(Utf8ToWCstring(li.Login),23);
    
    CurrentToolbar.SetImageList(m_PlaceSelectorImageList);
    CurrentToolbar.HideButton(IDC_LOGINTOOLBUTTON + ImageServer,(bool)!uploadEngine->NeedAuthorization);
    CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR1,(bool)!uploadEngine->NeedAuthorization);
    
    bool ShowLoginButton = !login.IsEmpty() && li.DoAuth;
    if(!ShowLoginButton)
    {
        if(uploadEngine->NeedAuthorization == 2)
            login = TR("Specify account...");
        else 
            login = TR("Account is not enabled");

    }
    bi.pszText = const_cast<LPWSTR>(login.GetString());
    CurrentToolbar.SetButtonInfo(IDC_LOGINTOOLBUTTON+ImageServer, &bi);

    bool ShowFolderButton = uploadEngine->SupportsFolders && ShowLoginButton;

    CurrentToolbar.HideButton(IDC_SELECTFOLDER,!ShowFolderButton);
    CurrentToolbar.HideButton(IDC_TOOLBARSEPARATOR2,!ShowFolderButton);
        
    CString title = WinUtils::TrimString(Utf8ToWCstring(serverProfile.folderTitle()), 27);
    if(title.IsEmpty()) title = TR("No Folder Selected");
    bi.pszText = const_cast<LPWSTR>(title.GetString());
    CurrentToolbar.SetButtonInfo(IDC_SELECTFOLDER, &bi);
    
}
void CUploadSettings::UpdateAllPlaceSelectors()
{
    UpdatePlaceSelector(false); // Update server selector (image hosting)
    UpdatePlaceSelector(true); // Update server selector (file hosting)
    UpdateToolbarIcons();
    updateMoreImageServersLink();
    updateMoreFileServersLink();
}

LRESULT CUploadSettings::OnImageServerSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nServerIndex = static_cast<int>(wID - IDC_IMAGESERVER_FIRST_ID);
    selectServer(getSessionImageServerItem(), nServerIndex);
    
    UpdateAllPlaceSelectors();
    return 0;
}

LRESULT CUploadSettings::OnFileServerSelect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    int nServerIndex = static_cast<int>(wID - IDC_FILESERVER_FIRST_ID);
    
    selectServer(getSessionFileServerItem(), nServerIndex);
    UpdateAllPlaceSelectors();
    return 0;
}
    
LRESULT CUploadSettings::OnServerDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    CMyEngineList* myEngineList = ServiceLocator::instance()->myEngineList();
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    auto* pnmtb = reinterpret_cast<NMTOOLBAR *>(pnmh);

    bool isImageServer = idCtrl == IDC_IMAGETOOLBAR;
    ServerProfile & serverProfile = isImageServer ? getSessionImageServerItem() : getSessionFileServerItem();
    std::vector<HBITMAP> bitmaps;
    CUploadEngineData *uploadEngine = nullptr;
    if(!serverProfile.isNull())
    {
        uploadEngine = serverProfile.uploadEngineData();
    }

    CToolBarCtrl& CurrentToolbar = isImageServer ? Toolbar: FileServerSelectBar;
    
    CMenu sub;    
    MENUITEMINFO mi;
    ZeroMemory(&mi,sizeof(mi));
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_TYPE|MIIM_ID;
    mi.fType = MFT_STRING;
    sub.CreatePopupMenu();

    if(pnmtb->iItem == IDC_SERVERBUTTON)
    {
        // Show popup menu with list of hostings
        int menuItemCount=0;
        int lastMenuBreakIndex = 0;
        bool nextItemBreaksLine = false;
        
        if(isImageServer)
        {
            for(int i=0; i<m_EngineList->count(); i++)
            {
                mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
                mi.fType = MFT_STRING;
                if(!m_EngineList->byIndex(i)->hasType(CUploadEngineData::TypeImageServer)) continue;
                mi.wID = (isImageServer ? IDC_IMAGESERVER_FIRST_ID: IDC_FILESERVER_FIRST_ID  ) +i;
                CUploadEngineData* ued = m_EngineList->byIndex(i);
                CString name  = Utf8ToWCstring(ued->Name); 
                mi.dwTypeData  = const_cast<LPWSTR>(name.GetString());
                mi.cch = name.GetLength();
                mi.hbmpItem = iconCache_->getIconBitmapForServer(ued->Name);

                if ( mi.hbmpItem ) {
                    mi.fMask |= MIIM_BITMAP;
                }
                if ( menuItemCount && (menuItemCount - lastMenuBreakIndex) % 34 == 0  ) {
                    mi.fType |= MFT_MENUBARBREAK ;
                    lastMenuBreakIndex = menuItemCount;
                }

                sub.InsertMenuItem(menuItemCount++, true, &mi);
            }

            ZeroMemory(&mi,sizeof(mi));
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_TYPE|MIIM_ID;
            mi.wID = IDC_FILESERVER_LAST_ID + 1;
            mi.fType = MFT_SEPARATOR;
            
            if ( menuItemCount && (menuItemCount - lastMenuBreakIndex) >= 23   ) {
                nextItemBreaksLine = true;
            } else {
                sub.InsertMenuItem(menuItemCount++, true, &mi);
            }
        }

        mi.fType = MFT_STRING;
        for(int i=0; i<m_EngineList->count(); i++)
        {
            mi.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
            mi.fType = MFT_STRING;
            if(!m_EngineList->byIndex(i)->hasType(CUploadEngineData::TypeFileServer)) continue;
            mi.wID = (isImageServer?IDC_IMAGESERVER_FIRST_ID: IDC_FILESERVER_FIRST_ID  ) +i;
            CUploadEngineData* ued = m_EngineList->byIndex(i);
            CString name  = Utf8ToWCstring(ued->Name); 
            mi.dwTypeData  = const_cast<LPWSTR>(name.GetString());
            mi.cch = name.GetLength();
            mi.hbmpItem = iconCache_->getIconBitmapForServer(ued->Name);
            if ( mi.hbmpItem ) {
                mi.fMask |= MIIM_BITMAP;
            }
            if ( menuItemCount && ((menuItemCount - lastMenuBreakIndex) % 34 == 0 || nextItemBreaksLine )  ) {

                mi.fType |= MFT_MENUBARBREAK ;
                lastMenuBreakIndex = menuItemCount;

            }
            nextItemBreaksLine = false;

            sub.InsertMenuItem(menuItemCount++, true, &mi);    
        }
        
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.wID = IDC_FILESERVER_LAST_ID + 1;
        mi.fType = MFT_SEPARATOR;
        /*if (menuItemCount && (menuItemCount - lastMenuBreakIndex) >= 30) {
            nextItemBreaksLine = true;
        }
        else*/
        {
            sub.InsertMenuItem(menuItemCount++, true, &mi);
        }
    
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
        mi.fType = MFT_STRING;
        mi.wID = isImageServer ? IDC_ADD_FTP_SERVER : IDC_ADD_FTP_SERVER_FROM_FILESERVER_LIST;
        if (nextItemBreaksLine)
        {
            nextItemBreaksLine = false;
            mi.fType |= MFT_MENUBARBREAK;
            lastMenuBreakIndex = menuItemCount;
        }
        CString addFtpServerStr = TR("Add FTP/SFTP server...");
        mi.dwTypeData = const_cast<LPWSTR>(addFtpServerStr.GetString());
        mi.cch = addFtpServerStr.GetLength();
        mi.hbmpItem = 0;
        sub.InsertMenuItem(menuItemCount++, true, &mi);    

        mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
        mi.fType = MFT_STRING;
        mi.wID = isImageServer ? IDC_ADD_DIRECTORY_AS_SERVER : IDC_ADD_DIRECTORY_AS_SERVER_FROM_FILESERVER_LIST;
        CString addLocalFolderStr = TR("Add local folder as new server...");
        mi.dwTypeData = const_cast<LPWSTR>(addLocalFolderStr.GetString());
        mi.cch = addLocalFolderStr.GetLength();
        mi.hbmpItem = 0;
        sub.InsertMenuItem(menuItemCount++, true, &mi);    

        sub.SetMenuDefaultItem(isImageServer ? 
            (IDC_IMAGESERVER_FIRST_ID + myEngineList->getUploadEngineIndex(Utf8ToWCstring(getSessionImageServerItem().serverName()))) :
            (IDC_FILESERVER_FIRST_ID + myEngineList->getUploadEngineIndex(Utf8ToWCstring(getSessionFileServerItem().serverName()))), FALSE);
    }
    else
    {
        // Show popup menu with list of accounts
        std::map <std::string, ServerSettingsStruct>& serverUsers = Settings.ServersSettings[serverProfile.serverName()];

        // Ignore account list if there is just one key "" in map (empty string, meaning no authentication at all)
        if(!serverUsers.empty() && (serverUsers.size() > 1 || serverUsers.find("") == serverUsers.end()) )
        {    
            bool addedSeparator = false;
            CAdvancedUploadEngine* plug = dynamic_cast<CAdvancedUploadEngine*>(uploadEngineManager_->getUploadEngine(serverProfile));
            /*if(!plug) return TBDDRET_TREATPRESSED;*/

            int i =0;
            if ( serverUsers.size() && !serverProfile.profileName().empty() ) {
                mi.wID = IDC_LOGINTOOLBUTTON + static_cast<int>(isImageServer);
                CString changeAccountSettingsStr = TR("Change account settings");
                mi.dwTypeData = const_cast<LPWSTR>(changeAccountSettingsStr.GetString());
                mi.cch = changeAccountSettingsStr.GetLength();
                sub.InsertMenuItem(i++, true, &mi);
            } else {
                addedSeparator = true;
            }

            menuOpenedUserNames_.clear();
            menuOpenedIsImageServer_ = isImageServer;

            if(plug && plug->supportsSettings()) {
                mi.wID = IDC_SERVERPARAMS + static_cast<int>(isImageServer);
                CString serverSettingsStr = TR("Server settings...");
                mi.dwTypeData = const_cast<LPWSTR>(serverSettingsStr.GetString());
                mi.cch = serverSettingsStr.GetLength();
                sub.InsertMenuItem(i++, true, &mi);
            }
            int command = IDC_USERNAME_FIRST_ID;
            HICON userIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICONUSER));

            for( const auto& it: serverUsers) {
                CString login = U2W(it.first);
                if (!login.IsEmpty() )/*&& it->second.authData.DoAuth**/ {
                    if ( !addedSeparator ) {
                        ZeroMemory(&mi,sizeof(mi));
                        mi.cbSize = sizeof(mi);
                        mi.fMask = MIIM_TYPE|MIIM_ID;
                        mi.wID = IDC_FILESERVER_LAST_ID + 1;
                        mi.fType = MFT_SEPARATOR;
        
                        sub.InsertMenuItem(i++, true, &mi);
                        addedSeparator =  true;
                    }
                    ZeroMemory(&mi,sizeof(mi));
                    mi.cbSize = sizeof(mi);
                
                    mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
                    mi.fType = MFT_STRING;
                    mi.wID = command;

                    mi.dwTypeData  = const_cast<LPWSTR>(login.GetString());
                    mi.cch = login.GetLength();
                    HBITMAP bm = iconBitmapUtils_->HIconToBitmapPARGB32(userIcon);
                    bitmaps.push_back(bm);
                    

                    mi.hbmpItem = bm;
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
                mi.wID = IDC_NO_ACCOUNT + !isImageServer;
                CString noAthenticationStr = TR("<no authentication>");
                mi.dwTypeData  = const_cast<LPWSTR>(noAthenticationStr.GetString());
                mi.cch = noAthenticationStr.GetLength();
                sub.InsertMenuItem(i++, true, &mi);
            }
  
            ZeroMemory(&mi,sizeof(mi));
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_TYPE|MIIM_ID;
            mi.wID = IDC_FILESERVER_LAST_ID + 1;
            mi.fType = MFT_SEPARATOR;

            sub.InsertMenuItem(i++, true, &mi);
            ZeroMemory(&mi,sizeof(mi));
            mi.cbSize = sizeof(mi);
            mi.fMask = MIIM_FTYPE |MIIM_ID | MIIM_STRING;
            mi.fType = MFT_STRING;
            mi.wID = IDC_ADD_ACCOUNT + !isImageServer;
            CString newAccountStr = TR("Add account...");
            mi.dwTypeData  = const_cast<LPWSTR>(newAccountStr.GetString());
            mi.cch = newAccountStr.GetLength();
            sub.InsertMenuItem(i++, true, &mi);
            sub.SetMenuDefaultItem(0,TRUE);
        }
        else
        {
            return TBDDRET_TREATPRESSED;
        }
    }
        
    RECT rc;
    CurrentToolbar.GetRect(pnmtb->iItem, &rc);
    CurrentToolbar.MapWindowPoints(nullptr, reinterpret_cast<LPPOINT>(&rc), 2);
    //CurrentToolbar.ClientToScreen(&rc);
    TPMPARAMS excludeArea;
    ZeroMemory(&excludeArea, sizeof(excludeArea));
    excludeArea.cbSize = sizeof(excludeArea);
    excludeArea.rcExclude = rc;
    sub.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, rc.left, rc.bottom, m_hWnd, &excludeArea);
    bHandled = true;
    for (auto bm: bitmaps) {
        DeleteObject(bm);
    }
    return TBDDRET_DEFAULT;
}

LRESULT CUploadSettings::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HWND hwnd = reinterpret_cast<HWND>(wParam);
    int xPos = LOWORD(lParam); 
    int yPos = HIWORD(lParam); 

    RECT rc;
    POINT pt = {xPos, yPos};

    if(hwnd == Toolbar.m_hWnd || hwnd == FileServerSelectBar.m_hWnd)
    {
        bool ImageServer = (hwnd == Toolbar.m_hWnd);
        CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

        ::SendMessage(CurrentToolbar.m_hWnd,TB_GETRECT, IDC_SERVERBUTTON, reinterpret_cast<LPARAM>(&rc));
        CurrentToolbar.ClientToScreen(&rc);
        if(PtInRect(&rc, pt))
        {
            OnServerButtonContextMenu(pt, ImageServer);
            return 0;
        }
        if(!CurrentToolbar.IsButtonHidden(IDC_SELECTFOLDER))
        {
            CurrentToolbar.GetRect(IDC_SELECTFOLDER, &rc);
            CurrentToolbar.ClientToScreen(&rc);
            if(PtInRect(&rc, pt))
            {
                OnFolderButtonContextMenu(pt, ImageServer);
                return 0;
            }
        }    
    }
    return 0;
}

void CUploadSettings::OnFolderButtonContextMenu(POINT pt, bool isImageServerToolbar)
{
    ServerProfile & serverProfile = isImageServerToolbar ? getSessionImageServerItem() : getSessionFileServerItem();

    CMenu sub;    
    sub.CreatePopupMenu();

    sub.AppendMenu(MFT_STRING, IDC_NEWFOLDER + static_cast<int>(isImageServerToolbar), TR("New folder"));

    if (!serverProfile.folderId().empty()) {
        UINT flags = MFT_STRING;
        if (serverProfile.folderId().empty() || serverProfile.folderId() == CFolderItem::NewFolderMark) {
            flags |= MFS_DISABLED;
        }
        sub.AppendMenu(flags, IDC_COPYFOLDERID + static_cast<int>(isImageServerToolbar), TR("Copy folder's ID"));
    }

    if (!serverProfile.folderUrl().empty()) {
        sub.AppendMenu(MFT_STRING, IDC_OPENINBROWSER + static_cast<int>(isImageServerToolbar), TR("Open in Web Browser"));
    }
            
    sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
}

LRESULT CUploadSettings::OnNewFolder(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    bool ImageServer = (wID % 2)!=0;
    ServerProfile & serverProfile = ImageServer ? getSessionImageServerItem() : getSessionFileServerItem();

    CScriptUploadEngine *m_pluginLoader = uploadEngineManager_->getScriptUploadEngine(serverProfile);
    if(!m_pluginLoader) return 0;

    std::vector<std::string> accessTypeList;

    m_pluginLoader->getAccessTypeList(accessTypeList);
    CFolderItem newFolder;

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = Settings->getServerSettings(serverProfile);

    if (serverProfile.folderId() == CFolderItem::NewFolderMark) {
        newFolder = serverSettings ? serverSettings->newFolder : CFolderItem();
    }

     CNewFolderDlg dlg(newFolder, true, accessTypeList);
     if(dlg.DoModal(m_hWnd) == IDOK)
     {
         serverProfile.setFolderTitle(newFolder.title);
         serverProfile.setFolderId(CFolderItem::NewFolderMark);
         serverProfile.setFolderUrl("");
         serverProfile.setParentIds(newFolder.parentIds);
         newFolder.setId(CFolderItem::NewFolderMark);

         if (serverSettings) {
             serverSettings->newFolder = newFolder;
         }
        UpdateAllPlaceSelectors();
     }
     return 0;
}
    
LRESULT CUploadSettings::OnOpenInBrowser(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    bool ImageServer = (wID % 2)!=0;
    ServerProfile & serverProfile = ImageServer? getSessionImageServerItem() : getSessionFileServerItem();

    CString str = U2W(serverProfile.folderUrl());
    if(!str.IsEmpty()) {
        WinUtils::ShellOpenFileOrUrl(str, m_hWnd);
    }
    return 0;
}

LRESULT CUploadSettings::OnCopyFolderId(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    bool ImageServer = (wID % 2) != 0;
    ServerProfile& serverProfile = ImageServer ? getSessionImageServerItem() : getSessionFileServerItem();
    std::string folderId = serverProfile.folderId();
    if (!folderId.empty() && folderId != CFolderItem::NewFolderMark) {
        CString str = U2W(folderId);
   
        WinUtils::CopyTextToClipboard(str);
    }
    return 0;
}
    
void CUploadSettings::OnServerButtonContextMenu(POINT pt, bool isImageServerToolbar)
{
    ServerProfile & serverProfile = isImageServerToolbar? getSessionImageServerItem() : getSessionFileServerItem();
    if ( serverProfile.isNull() ) {
        return;
    }
    int iconWidth = ::GetSystemMetrics(SM_CXSMICON);
    int iconHeight = ::GetSystemMetrics(SM_CYSMICON);
    CIcon ico;
    ico.LoadIconWithScaleDown(MAKEINTRESOURCE(IDI_ICONSETTINGS), iconWidth, iconHeight);
    
    CMenu sub;    
    sub.CreatePopupMenu();
    MENUITEMINFO mi {};
    mi.cbSize = sizeof(mi);
    mi.fMask = MIIM_ID | MIIM_STRING | MIIM_BITMAP;
    mi.wID = IDC_SERVERPARAMS + (int)isImageServerToolbar;
    CString serverSettingsStr = TR("Server settings");
    CBitmap bm = iconBitmapUtils_->HIconToBitmapPARGB32(ico);
    mi.hbmpItem = bm;
    mi.dwTypeData = const_cast<LPWSTR>(serverSettingsStr.GetString());
    mi.cch = serverSettingsStr.GetLength();
    if (!sub.InsertMenuItem(0, true, &mi)) {
        //LOG(WARNING) << "Cannot insert menu item" << std::endl << WinUtils::GetLastErrorAsString();
    }

    mi.fMask = MIIM_ID | MIIM_STRING;
    if(!serverProfile.uploadEngineData()->RegistrationUrl.empty())
    {
        mi.wID = IDC_OPENREGISTERURL + (int)isImageServerToolbar;
        CString goToSignupPageStr = TR("Go to signup page");
        mi.dwTypeData = const_cast<LPWSTR>(goToSignupPageStr.GetString());
        mi.cch = goToSignupPageStr.GetLength();
        sub.InsertMenuItem(1, true, &mi);
    }

    if (!serverProfile.uploadEngineData()->WebsiteUrl.empty()) {
        sub.AppendMenu(MF_STRING, IDC_OPENWEBSITE + (int)isImageServerToolbar, TR("Open the website"));
    }
    sub.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, pt.x, pt.y, m_hWnd);
}

LRESULT CUploadSettings::OnServerParamsClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    bool ImageServer = (wID % 2)!=0;
    //CToolBarCtrl& CurrentToolbar = (ImageServer) ? Toolbar: FileServerSelectBar;

    ServerProfile& serverProfile = ImageServer ? getSessionImageServerItem() : getSessionFileServerItem();
    CUploadEngineData *ue = serverProfile.uploadEngineData();
    if (!ue->UsingPlugin && ue->Engine.empty()) {
        GuiTools::LocalizedMessageBox(m_hWnd, TR("This server doesn't have any settings."), APPNAME, MB_ICONINFORMATION);
        return false;
    }

    CServerParamsDlg dlg(serverProfile, uploadEngineManager_);
    if ( dlg.DoModal(m_hWnd) == IDOK) {
        serverProfile = dlg.serverProfile();
        UpdateAllPlaceSelectors();
    }
    return 0;
}

LRESULT CUploadSettings::OnOpenSignupPage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    bool ImageServer = (wID % 2)!=0;
    ServerProfile & serverProfile = ImageServer? getSessionImageServerItem() : getSessionFileServerItem();

    CUploadEngineData *ue = serverProfile.uploadEngineData();
    if (ue && !ue->RegistrationUrl.empty()) {
        WinUtils::ShellOpenFileOrUrl(U2W(ue->RegistrationUrl), m_hWnd);
    }
    return 0;
}

LRESULT CUploadSettings::OnResizePresetButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    RECT rc;
    ::GetWindowRect(hWndCtl, &rc);
    POINT menuOrigin = {rc.left,rc.bottom};

    CMenu FolderMenu;
    int id = IDC_RESIZEPRESETMENU_FIRST_ID;
    FolderMenu.CreatePopupMenu();

    int prevMenuGroup = 0;
    for ( const auto& preset: resizePresets) {
        // Add menu separator if group number has changed
        if (preset.menuGroup != prevMenuGroup) {
            FolderMenu.AppendMenu(MF_SEPARATOR, static_cast<UINT_PTR>(-1), _T(""));
            prevMenuGroup = preset.menuGroup;
        }
        if (preset.width.empty() && preset.height.empty()) {
            FolderMenu.AppendMenu(MF_STRING, id++, TR("Don't resize"));
            continue;
        } 

        std::wstring menuItemTitle;
        if (preset.title.empty()) {
            // Format menu item text like "WxH" 
            menuItemTitle  = str(boost::wformat(L"%s\u00D7%s") % preset.width % preset.height);
        } else {
            menuItemTitle = preset.title;
        }

        FolderMenu.AppendMenu(MF_STRING, id++, menuItemTitle.c_str());
    }
   
    FolderMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, menuOrigin.x, menuOrigin.y, m_hWnd);
    
   return 0;
}

LRESULT CUploadSettings::OnShorteningUrlServerButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    CServerSelectorControl serverSelectorControl(uploadEngineManager_, false, false);
    serverSelectorControl.setServersMask(CServerSelectorControl::smUrlShorteners);
    serverSelectorControl.setShowImageProcessingParams(false);
    serverSelectorControl.setTitle(TR("URL shortening server"));
    serverSelectorControl.setServerProfile(Settings.urlShorteningServer);
    RECT clientRect;
    m_ShorteningServerButton.GetClientRect(&clientRect);
    m_ShorteningServerButton.ClientToScreen(&clientRect);
    POINT pt = { clientRect.left, clientRect.bottom };
    serverSelectorControl.setOnChangeCallback(std::bind(&CUploadSettings::shorteningUrlServerChanged, this, std::placeholders::_1));
    serverSelectorControl.showPopup(m_hWnd, pt);
    Settings.urlShorteningServer = serverSelectorControl.serverProfile();
    updateUrlShorteningCheckboxLabel();
    return 0;
}

LRESULT CUploadSettings::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
	::DestroyWindow(useServerThumbnailsTooltip_);
	useServerThumbnailsTooltip_ = nullptr;
	return 0;
}

LRESULT CUploadSettings::OnResizePresetMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
   int presetIndex = wID - IDC_RESIZEPRESETMENU_FIRST_ID;
   int presetCount = ARRAY_SIZE(resizePresets);
   if (presetIndex > presetCount - 1) {
       return 0;
   }
   ::SetDlgItemText(m_hWnd, IDC_IMAGEWIDTH, resizePresets[presetIndex].width.c_str());
   ::SetDlgItemText(m_hWnd, IDC_IMAGEHEIGHT, resizePresets[presetIndex].height.c_str());
   return 0;
}
LRESULT CUploadSettings::OnEditProfileClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
   SaveCurrentProfile();
   CSettingsDlg dlg(CSettingsDlg::spImages, uploadEngineManager_);
   dlg.DoModal(m_hWnd);
   CurrentProfileName.Empty();
   ShowParams(U2W(sessionImageServer_.getByIndex(0).getImageUploadParamsRef().ImageProfileName));
   UpdateProfileList();
   ShowParams(U2W(sessionImageServer_.getByIndex(0).getImageUploadParamsRef().ImageProfileName));
   return 0;
}

 void CUploadSettings::UpdateProfileList()
 {
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_RESETCONTENT);
    std::map<CString, ImageConvertingParams> ::const_iterator it;
    bool found = false;
    for(it = convert_profiles_.begin(); it!=convert_profiles_.end(); ++it)
    {
      GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, it->first);
      if(it->first == CurrentProfileName) found = true;
    }
    if (!found) {
        GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, CurrentProfileName);
    }
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, static_cast<WPARAM>(-1),(LPARAM)(LPCTSTR) CurrentProfileName); 
 }

 void CUploadSettings::selectServer(ServerProfile& sp, int serverIndex)
 {
     CMyEngineList* myEngineList = ServiceLocator::instance()->myEngineList();
     WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
     sp.setServerName(myEngineList->byIndex(serverIndex)->Name);
     std::map <std::string, ServerSettingsStruct>& serverSettings = Settings.ServersSettings[sp.serverName()];
     std::map <std::string, ServerSettingsStruct>::iterator firstAccount = serverSettings.begin();
     if ( firstAccount != serverSettings.end() ) {
         if ( firstAccount->first == "" ) {
             ++firstAccount;
         }
         if ( firstAccount != serverSettings.end() ) {
             sp.setProfileName(firstAccount->first);
         }
     } else {
         sp.setProfileName("");
     }
     ServerSettingsStruct* ss = Settings.getServerSettings(sp);
     sp.setFolderId(ss ? ss->defaultFolder.getId() : std::string());
     sp.setFolderTitle(ss ? ss->defaultFolder.getTitle(): std::string());
     sp.setFolderUrl(ss ? ss->defaultFolder.viewUrl: std::string());
     sp.setParentIds(ss ? ss->defaultFolder.parentIds : std::vector<std::string>());
 }

void CUploadSettings::updateUrlShorteningCheckboxLabel()
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    CString text;
    CString serverName = Utf8ToWCstring(Settings.urlShorteningServer.serverName());
    text.Format(TR("Shorten URL using %s"), static_cast<LPCTSTR>(serverName));
    SetDlgItemText(IDC_SHORTENLINKSCHECKBOX, text);
}

void CUploadSettings::shorteningUrlServerChanged(CServerSelectorControl* serverSelectorControl)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    Settings.urlShorteningServer = serverSelectorControl->serverProfile();
    updateUrlShorteningCheckboxLabel();
}

void CUploadSettings::ShowParams(const ImageConvertingParams& params)
{
    m_ProfileChanged = false;
    m_CatchChanges = false;
    if (params.Quality) {
        SetDlgItemInt(IDC_QUALITYEDIT, params.Quality);
    } else {
        SetDlgItemText(IDC_QUALITYEDIT, _T(""));
    }

    SendDlgItemMessage(IDC_FORMATLIST,CB_SETCURSEL, params.Format);
    SendDlgItemMessage(IDC_YOURLOGO,BM_SETCHECK, params.AddLogo);
    SendDlgItemMessage(IDC_YOURTEXT,BM_SETCHECK, params.AddText);
    SetDlgItemText(IDC_IMAGEWIDTH,U2W(params.strNewWidth));
    SetDlgItemText(IDC_IMAGEHEIGHT,U2W(params.strNewHeight));
    SendDlgItemMessage(IDC_SKIPANIMATEDCHECKBOX, BM_SETCHECK, params.SkipAnimated ? BST_CHECKED : BST_UNCHECKED);
    m_ProfileChanged = false;
    m_CatchChanges = true;
}

void CUploadSettings::ShowParams(const CString& profileName) {
    if (convert_profiles_.find(profileName) == convert_profiles_.end()) {
        return;
    }
    auto thumb = getSessionImageServerItem().getImageUploadParams().getThumb();

    SetDlgItemText(IDC_THUMBWIDTH, L"");
    SetDlgItemText(IDC_THUMBHEIGHT, L"");

    if (thumb.Width && (thumb.ResizeMode == ThumbCreatingParams::trByWidth || thumb.ResizeMode == ThumbCreatingParams::trByBoth)){
        SetDlgItemInt(IDC_THUMBWIDTH, thumb.Width);
    }
    if (thumb.Height && (thumb.ResizeMode == ThumbCreatingParams::trByHeight || thumb.ResizeMode == ThumbCreatingParams::trByBoth)) {
        SetDlgItemInt(IDC_THUMBHEIGHT, thumb.Height);
    }

    if (CurrentProfileName == profileName) {
        return;
    }
    CurrentProfileName = profileName;
    CurrentProfileOriginalName = profileName; 
    ShowParams(convert_profiles_[profileName]);
   
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_SELECTSTRING, static_cast<WPARAM>(-1),(LPARAM)(LPCTSTR) profileName); 
 }

bool CUploadSettings::SaveParams(ImageConvertingParams& params) {
    params.Quality = GetDlgItemInt(IDC_QUALITYEDIT);
    params.Format = SendDlgItemMessage(IDC_FORMATLIST, CB_GETCURSEL);

    params.strNewWidth = W2U(GuiTools::GetWindowText( GetDlgItem(IDC_IMAGEWIDTH)));

    params.strNewHeight = W2U(GuiTools::GetWindowText( GetDlgItem(IDC_IMAGEHEIGHT)));
    params.SkipAnimated = GuiTools::GetCheck(m_hWnd, IDC_SKIPANIMATEDCHECKBOX);
    return true;
}

void CUploadSettings::ProfileChanged() {
    if (!m_CatchChanges) return;
    if (!m_ProfileChanged) {
        CurrentProfileOriginalName = CurrentProfileName;
        CurrentProfileName.Replace(CString(_T(" ")) + TR("(edited)"), _T(""));
        CurrentProfileName = CurrentProfileName + _T(" ") + TR("(edited)");
        // ::SetWindowText(GetDlgItem(IDC_PROFILECOMBO), CurrentProfileName);
        m_ProfileChanged = true;
        UpdateProfileList();
    }
}

LRESULT CUploadSettings::OnProfileEditedCommand(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
   ProfileChanged();
   return 0;
}

LRESULT CUploadSettings::OnUserNameMenuItemClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    int userNameIndex = wID - IDC_USERNAME_FIRST_ID;
    CString userName = menuOpenedUserNames_[userNameIndex];
    bool ImageServer = menuOpenedIsImageServer_;
    ServerProfile & serverProfile = ImageServer? getSessionImageServerItem() : getSessionFileServerItem();
    serverProfile.setProfileName(WCstringToUtf8(userName));

    BasicSettings* Settings = ServiceLocator::instance()->basicSettings();
    ServerSettingsStruct* serverSettings = Settings->getServerSettings(serverProfile);

    serverProfile.setFolderId(serverSettings ? serverSettings->defaultFolder.getId(): std::string());
    serverProfile.setFolderTitle(serverSettings ? serverSettings->defaultFolder.getTitle(): std::string());
    serverProfile.setFolderUrl(serverSettings ? serverSettings->defaultFolder.viewUrl: std::string());
    serverProfile.setParentIds(serverSettings ? serverSettings->defaultFolder.parentIds : std::vector<std::string>());
    /*if(UserName != ss.authData.Login || ss.authData.DoAuth!=prevAuthEnabled)
    {
        serverProfile.setFolderId("");
        serverProfile.setFolderTitle("");
        serverProfile.setFolderUrl("");
        iuPluginManager.UnloadPlugins();
        m_EngineList->DestroyCachedEngine(m_EngineList->byIndex(nServerIndex)->Name);
    }*/

    UpdateAllPlaceSelectors();
    return 0;
}

LRESULT CUploadSettings::OnAddAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bool isImageServer = (wID % 2)!=0;

    ServerProfile & serverProfile = isImageServer? getSessionImageServerItem() : getSessionFileServerItem();
    ServerProfile serverProfileCopy = serverProfile;
    serverProfileCopy.setProfileName("");
    CLoginDlg dlg(serverProfileCopy, uploadEngineManager_, true);
    UINT dlgResult = dlg.DoModal(m_hWnd);
    //ServerSettingsStruct & ss = ImageServer ? sessionImageServer_.serverSettings() : sessionFileServer_.serverSettings();
    if (dlgResult  != IDCANCEL) {
        serverProfileCopy.setProfileName(WCstringToUtf8(dlg.accountName()));
        serverProfileCopy.setFolderId("");
        serverProfileCopy.setFolderTitle("");
        serverProfileCopy.setFolderUrl("");
        serverProfileCopy.setParentIds({});
        serverProfile = serverProfileCopy;
        UpdateAllPlaceSelectors();
    } 
    return 0;
}

LRESULT CUploadSettings::OnNoAccountClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bool ImageServer = (wID % 2)!=0;
    ServerProfile & serverProfile = ImageServer? getSessionImageServerItem() : getSessionFileServerItem();
    serverProfile.setProfileName("");
    serverProfile.setFolderId("");
    serverProfile.setFolderTitle("");
    serverProfile.setFolderUrl("");
    serverProfile.setParentIds({});
    UpdateAllPlaceSelectors();
    return 0;
}

void CUploadSettings::SaveCurrentProfile()
{
    CString saveToProfile = CurrentProfileName;
    if (CurrentProfileOriginalName == _T("Default"))
        saveToProfile = CurrentProfileOriginalName;

    if (!SaveParams(convert_profiles_[saveToProfile]))
        return;

    getSessionImageServerItem().getImageUploadParamsRef().ImageProfileName = W2U(saveToProfile);
}

bool  CUploadSettings::OnHide()
{
   SaveCurrentProfile();
   return true;
}
LRESULT CUploadSettings::OnProfileComboSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    CString profile = GuiTools::GetWindowText(GetDlgItem(IDC_PROFILECOMBO));

    ShowParams(profile);
    UpdateProfileList();
   return 0;
}


LRESULT CUploadSettings::OnAddFtpServer(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    CAddFtpServerDialog dlg(m_EngineList);
    if (dlg.DoModal(m_hWnd) == IDOK) {
        
        if (wID == IDC_ADD_FTP_SERVER) {
            auto& sessionImageServer = getSessionImageServerItem();
            sessionImageServer.setServerName(WCstringToUtf8(dlg.createdServerName()));
            sessionImageServer.setProfileName(WCstringToUtf8(dlg.createdServerLogin()));
        } else {
            auto& sessionFileServer = getSessionFileServerItem();
            sessionFileServer.setServerName(WCstringToUtf8(dlg.createdServerName()));
            sessionFileServer.setProfileName(WCstringToUtf8(dlg.createdServerLogin()));
        }

        UpdateAllPlaceSelectors();
    }
    return 0;
}

LRESULT CUploadSettings::OnAddDirectoryAsServer(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
    CAddDirectoryServerDialog dlg(m_EngineList);
    if ( dlg.DoModal(m_hWnd) == IDOK ) {
        if (wID == IDC_ADD_DIRECTORY_AS_SERVER) {
            auto& sessionImageServer = getSessionImageServerItem();
            sessionImageServer.setServerName(WCstringToUtf8(dlg.createdServerName()));
            sessionImageServer.setProfileName("");
            sessionImageServer.clearFolderInfo();
        } else {
            auto& sessionFileServer = getSessionFileServerItem();
            sessionFileServer.setServerName(WCstringToUtf8(dlg.createdServerName()));
            sessionFileServer.setProfileName("");
            sessionFileServer.clearFolderInfo();
        }

        UpdateAllPlaceSelectors();
    }
    return 0;
}


ServerProfile& CUploadSettings::getSessionImageServerItem() {
    return sessionImageServer_.getByIndex(0);
}
ServerProfile& CUploadSettings::getSessionFileServerItem() {
    return sessionFileServer_.getByIndex(0);
}

LRESULT CUploadSettings::OnChooseMoreImageServersClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CServerProfileGroupSelectDialog dlg(uploadEngineManager_, sessionImageServer_, CServerSelectorControl::smImageServers | CServerSelectorControl::smFileServers);
    if (dlg.DoModal(m_hWnd) == IDOK) {
        sessionImageServer_ = dlg.serverProfileGroup();
        
        UpdateAllPlaceSelectors();
    }
    return 0;
}

LRESULT CUploadSettings::OnChooseMoreFileServersClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    CServerProfileGroupSelectDialog dlg(uploadEngineManager_, sessionFileServer_, CServerSelectorControl::smFileServers);
    if (dlg.DoModal(m_hWnd) == IDOK) {
        sessionFileServer_ = dlg.serverProfileGroup();

        UpdateAllPlaceSelectors();
    }
    return 0;
}

LRESULT CUploadSettings::OnOpenWebsite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    bool ImageServer = (wID % 2) != 0;
    ServerProfile& serverProfile = ImageServer ? getSessionImageServerItem() : getSessionFileServerItem();

    CUploadEngineData* ue = serverProfile.uploadEngineData();
    if (ue && !ue->WebsiteUrl.empty()) {
        WinUtils::ShellOpenFileOrUrl(U2W(ue->WebsiteUrl), m_hWnd);
    }
    return  0;
}

void CUploadSettings::updateMoreImageServersLink() {
    std::wstring text;
    if (sessionImageServer_.getCount() == 1) {
        text = TR("Choose more servers...");
    } else {
        text = str(boost::wformat(TR("Selected servers: %d"))% sessionImageServer_.getCount());
    }
        
    moreImageServersLink_.SetLabel(text.c_str());
}


void CUploadSettings::updateMoreFileServersLink() {
    std::wstring text;
    if (sessionFileServer_.getCount() == 1) {
        text = TR("Choose more servers...");
    }
    else {
        text = str(boost::wformat(TR("Selected servers: %d")) % sessionFileServer_.getCount());
    }

    moreFileServersLink_.SetLabel(text.c_str());
}
