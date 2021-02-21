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

#include "UploadParamsDlg.h"

#include "Gui/Dialogs/wizarddlg.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Func/IuCommonFunctions.h"

// CUploadParamsDlg
CUploadParamsDlg::CUploadParamsDlg(ServerProfile &serverProfile, bool showImageProcessingParams, bool defaultServer) : serverProfile_(serverProfile)
{
    params_ = serverProfile.getImageUploadParams();
    defaultServer_ = defaultServer;
    showImageProcessingParams_ = showImageProcessingParams;
    m_UploadEngine = nullptr;
}

CUploadParamsDlg::~CUploadParamsDlg()
{
    
}

LRESULT CUploadParamsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    WtlGuiSettings& Settings = *ServiceLocator::instance()->settings<WtlGuiSettings>();
    CenterWindow(GetParent());

    ThumbBackground_.SubclassWindow(GetDlgItem(IDC_THUMBBACKGROUND));
    ThumbBackground_.SetColor(Settings.imageServer.getImageUploadParams().getThumb().BackgroundColor);

    SetWindowText(TR("Image processing"));
    TRC(IDC_DEFAULTSETTINGSCHECKBOX, "Default settings");
    TRC(IDC_IMAGEPARAMETERS, "Image settings");
    TRC(IDC_PROCESSIMAGESCHECKBOX, "Process images before upload");
    TRC(IDC_PROFILELABEL, "Profile:");
    TRC(IDC_THUMBSETTINGS, "Thumbnail settings");
    TRC(IDC_CREATETHUMBNAILS, "Create thumbnails");
    TRC(IDC_USESERVERTHUMBNAILS, "Use server-side thumbnails");
    TRC(IDC_THUMBTEMPLATECOMBOLABEL, "Thumbnail Preset:");
    TRC(IDC_THUMBRESIZELABEL, "Scaling:");
    TRC(IDC_DEFAULTTHUMBSETTINGSCHECKBOX, "Use default thumbnail settings");
    TRC(IDC_SHORTENLINKSCHECKBOX, "Shorten URLs");
    TRC(IDCANCEL, "Cancel");
    TRC(IDC_THUMBFORMATLABEL, "Format:");
    TRC(IDC_THUMBQUALITYLABEL, "Quality:");
    TRC(IDC_THUMBTEXTCHECKBOX, "Thumbnail text:");
    TRC(IDC_THUMBBACKGROUNDLABEL, "Background color:");

    profileCombo_ = GetDlgItem(IDC_PROFILECOMBO);
    thumbTemplateCombo_ = GetDlgItem(IDC_THUMBTEMPLATECOMBO);
    //Fill profile combobox
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_RESETCONTENT);

    int selectedIndex = -1;
    int i = 0;
    for (auto it = Settings.ConvertProfiles.begin(); it != Settings.ConvertProfiles.end(); ++it) {
        GuiTools::AddComboBoxItem(m_hWnd, IDC_PROFILECOMBO, it->first);
        if (it->first == U2W(serverProfile_.getImageUploadParams().ImageProfileName)) {

            selectedIndex = i;
        }
        i++;

    }
    SendDlgItemMessage(IDC_PROFILECOMBO, CB_SETCURSEL, selectedIndex,0);

    // Fill thumb profiles
    std::vector<CString> files;
    CString folder = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\");
    WinUtils::GetFolderFileList(files, folder , _T("*.xml"));
    for(size_t i=0; i<files.size(); i++) {
        GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBTEMPLATECOMBO, 1, Utf8ToWCstring(IuCoreUtils::ExtractFileNameNoExt( WCstringToUtf8( files[i]))) );
    }
    SendDlgItemMessage(IDC_THUMBTEMPLATECOMBO, CB_SELECTSTRING, static_cast<WPARAM>(-1),(LPARAM)(LPCTSTR) U2W(params_.getThumbRef().TemplateName)); 

    GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBFORMATLIST, 4, TR("Same format as image"),
        _T("JPEG"), _T("PNG"), _T("GIF"));

    GuiTools::AddComboBoxItems(m_hWnd, IDC_THUMBRESIZECOMBO, 2, 
        TR("By width"), TR("By height"), TR("By larger side") );
    SendDlgItemMessage(IDC_THUMBRESIZECOMBO, CB_SETCURSEL, (int)params_.getThumbRef().ResizeMode,0);

    GuiTools::SetCheck(m_hWnd, IDC_PROCESSIMAGESCHECKBOX, params_.ProcessImages);
    GuiTools::SetCheck(m_hWnd, IDC_CREATETHUMBNAILS, params_.CreateThumbs);
    GuiTools::SetCheck(m_hWnd, IDC_USESERVERTHUMBNAILS, params_.UseServerThumbs);
    GuiTools::SetCheck(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX, params_.UseDefaultThumbSettings);
    if ( defaultServer_ ) {
        serverProfile_.UseDefaultSettings = false;
        GuiTools::ShowDialogItem(m_hWnd,IDC_DEFAULTSETTINGSCHECKBOX, false);
    }
    GuiTools::SetCheck(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX, serverProfile_.UseDefaultSettings || !showImageProcessingParams_ );
    if (!showImageProcessingParams_)
    {
        GuiTools::EnableDialogItem(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX, false);
    }
    GuiTools::SetCheck(m_hWnd, IDC_THUMBTEXTCHECKBOX,params_.getThumbRef().AddImageSize);
    GuiTools::SetCheck(m_hWnd, IDC_SHORTENLINKSCHECKBOX, serverProfile_.shortenLinks());

    SetDlgItemText(IDC_THUMBTEXT, U2W(params_.getThumbRef().Text) );

    SetDlgItemText(IDC_WIDTHEDIT, WinUtils::IntToStr(params_.getThumbRef().Size));
        SetDlgItemInt(IDC_THUMBQUALITYEDIT,  params_.getThumbRef().Quality);
    SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_SETCURSEL, static_cast<int>( (params_.getThumbRef().Format)), 0);

    //GuiTools::SetCheck(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX, params_.);

    SendDlgItemMessage(IDC_THUMBQUALITYSPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );    

    createThumbnailsCheckboxChanged();
    processImagesChanged();
    defaultSettingsCheckboxChanged();
    return 0;  
}

LRESULT CUploadParamsDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    if (showImageProcessingParams_) {
        GuiTools::GetCheck(m_hWnd, IDC_PROCESSIMAGESCHECKBOX, params_.ProcessImages);
        GuiTools::GetCheck(m_hWnd, IDC_CREATETHUMBNAILS, params_.CreateThumbs);
        GuiTools::GetCheck(m_hWnd, IDC_USESERVERTHUMBNAILS, params_.UseServerThumbs);
        GuiTools::GetCheck(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX, params_.UseDefaultThumbSettings);
        GuiTools::GetCheck(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX, serverProfile_.UseDefaultSettings);
        params_.getThumbRef().AddImageSize = GuiTools::GetCheck(m_hWnd, IDC_THUMBTEXTCHECKBOX);
        params_.getThumbRef().Text = W2U(GuiTools::GetDlgItemText(m_hWnd, IDC_THUMBTEXT));
        
        int profileIndex = profileCombo_.GetCurSel();
        CString buf;
        profileCombo_.GetLBText(profileIndex, buf);
        params_.ImageProfileName = W2U(buf);
        params_.getThumbRef().Size = GetDlgItemInt(IDC_WIDTHEDIT);

        CString buf2;
        profileIndex = thumbTemplateCombo_.GetCurSel();
        thumbTemplateCombo_.GetLBText(profileIndex, buf2);
        ThumbCreatingParams& Thumb = params_.getThumbRef();
        Thumb.TemplateName = W2U(buf2);

        Thumb.ResizeMode = static_cast<ThumbCreatingParams::ThumbResizeEnum>(SendDlgItemMessage(IDC_THUMBRESIZECOMBO, CB_GETCURSEL, 0, 0));

        Thumb.Format = static_cast<ThumbCreatingParams::ThumbFormatEnum>(SendDlgItemMessage(IDC_THUMBFORMATLIST, CB_GETCURSEL));
        Thumb.Quality = GetDlgItemInt(IDC_THUMBQUALITYEDIT);
        Thumb.BackgroundColor = ThumbBackground_.GetColor();
    }
    serverProfile_.setShortenLinks(GuiTools::GetCheck(m_hWnd, IDC_SHORTENLINKSCHECKBOX));


    EndDialog(wID);
    return 0;
}

LRESULT CUploadParamsDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CUploadParamsDlg::OnClickedCreateThumbnailsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    createThumbnailsCheckboxChanged();
    return 0;
}

void CUploadParamsDlg::createThumbnailsCheckboxChanged() {
    bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_CREATETHUMBNAILS);
    GuiTools::EnableNextN(GetDlgItem(IDC_CREATETHUMBNAILS), isChecked? 4 : 18, isChecked );
    
    if ( isChecked ) {
        defaultThumbSettingsCheckboxChanged();
        
    }
    thumbTextCheckboxChanged();
}

void CUploadParamsDlg::processImagesChanged() {
    bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_PROCESSIMAGESCHECKBOX);
    GuiTools::EnableNextN(GetDlgItem(IDC_PROCESSIMAGESCHECKBOX), 2, isChecked );
}

LRESULT CUploadParamsDlg::OnClickedProcessImagesCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    processImagesChanged();
    return 0;
}

void CUploadParamsDlg::defaultSettingsCheckboxChanged() {
    bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX);
    GuiTools::EnableNextN(GetDlgItem(IDC_DEFAULTSETTINGSCHECKBOX), 17, !isChecked );
    
    if ( !isChecked ) {

        createThumbnailsCheckboxChanged();
        processImagesChanged();
        
    }
}

LRESULT CUploadParamsDlg::OnClickedDefaultSettingsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)  {
    defaultSettingsCheckboxChanged();
    return 0;
}

LRESULT  CUploadParamsDlg::OnClickedDefaultThumbSettingsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    defaultThumbSettingsCheckboxChanged();
    return 0;
}

void  CUploadParamsDlg::defaultThumbSettingsCheckboxChanged() {
    bool useDefaultThumbnailSettings = GuiTools::IsChecked(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX);
    //GuiTools::EnableNextN(GetDlgItem(IDC_DEFAULTTHUMBSETTINGSCHECKBOX), 8, !useDefaultThumbnailSettings );
    bool useDefaultSettings = GuiTools::IsChecked(m_hWnd, IDC_DEFAULTSETTINGSCHECKBOX);
    bool useServerThumbnails = GuiTools::IsChecked(m_hWnd, IDC_USESERVERTHUMBNAILS);

    bool addThumbText = GuiTools::IsChecked(m_hWnd, IDC_THUMBTEXTCHECKBOX);

    std::map<int, bool> enable;
    enable[IDC_THUMBTEMPLATECOMBOLABEL] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBTEMPLATECOMBO] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    //enable[IDC_THUMBRESIZECOMBO] = !useServerThumbnails && !useDefaultSettings;
    enable[IDC_THUMBTEXTCHECKBOX] = !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBTEXT] = !useDefaultThumbnailSettings && addThumbText && !useDefaultSettings;
    enable[IDC_THUMBRESIZELABEL] = !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_WIDTHEDIT] = !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_WIDTHEDITUNITS] = !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBRESIZECOMBO] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBFORMATLABEL] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBFORMATLIST] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBBACKGROUNDLABEL] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBBACKGROUND] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBQUALITYLABEL] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBQUALITYEDIT] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_THUMBQUALITYSPIN] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;
    enable[IDC_PERCENTLABEL2] = !useServerThumbnails && !useDefaultThumbnailSettings && !useDefaultSettings;

    for ( std::map<int,bool>::const_iterator it = enable.begin(); it != enable.end(); ++it) {
        GuiTools::EnableDialogItem(m_hWnd, it->first, it->second);
    }
}

void  CUploadParamsDlg::thumbTextCheckboxChanged() {
    bool createThumbnails = GuiTools::IsChecked(m_hWnd, IDC_CREATETHUMBNAILS);
    bool isChecked = GuiTools::IsChecked(m_hWnd, IDC_THUMBTEXTCHECKBOX);
    bool useDefaultThumbnailSettings = GuiTools::IsChecked(m_hWnd, IDC_DEFAULTTHUMBSETTINGSCHECKBOX);
    GuiTools::EnableDialogItem(m_hWnd, IDC_THUMBTEXT, isChecked && !useDefaultThumbnailSettings && createThumbnails);
}

LRESULT CUploadParamsDlg::OnClickedThumbTextCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    thumbTextCheckboxChanged();
    return 0;
}

LRESULT CUploadParamsDlg::OnClickedUseServerThumbnailsCheckbox(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    useServerThumbnailsChanged();
    return 0;
}

void CUploadParamsDlg::useServerThumbnailsChanged() {
    bool useServerThumbnails = GuiTools::IsChecked(m_hWnd, IDC_USESERVERTHUMBNAILS);

    if ( useServerThumbnails ) {
        // Select item 'resize by width'
        SendDlgItemMessage(IDC_THUMBRESIZECOMBO, CB_SETCURSEL, 0, 0);
    }

    defaultThumbSettingsCheckboxChanged();
}

ImageUploadParams CUploadParamsDlg::imageUploadParams() const {
    return params_;
}