/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "MediaInfoDlg.h"

#include "Func/MediaInfoHelper.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Core/Settings.h"

// CMediaInfoDlg
CMediaInfoDlg::CMediaInfoDlg()
{
    infoType_ = static_cast<InfoType>(Settings.MediaInfoSettings.InfoType);
    generateTextInEnglish_ = !Settings.MediaInfoSettings.EnableLocalization;
}

CMediaInfoDlg::~CMediaInfoDlg()
{
    
}

LRESULT CMediaInfoDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    GuiTools::MakeLabelBold(GetDlgItem(IDC_FILEINFOLABEL));
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"
    LOGFONT logFont;
    WinUtils::StringToFont(_T("Courier New,8,,204"), &logFont);
    
    editFont_.CreateFontIndirect(&logFont);
    SendDlgItemMessage(IDC_FILEINFOEDIT, WM_SETFONT, reinterpret_cast<WPARAM>(editFont_.m_hFont), MAKELPARAM(false, 0));
    editControl_.AttachToDlgItem(m_hWnd, IDC_FILEINFOEDIT);

    // Translating controls' text
    TRC(IDOK, "OK");
    SetWindowText(TR("Information about file"));
    TRC(IDC_COPYALL, "Copy to clipboard");
    TRC(IDC_SUMMARYRADIOBUTTON, "Summary");
    TRC(IDC_FULLINFORADIOBUTTON, "Full information");
    TRC(IDC_GENERATETEXTINENGLISHCHECKBOX, "Disable localization");

    SetDlgItemText(IDC_FILEINFOEDIT, TR("Loading..."));

    if (infoType_ == itFullInformation) {
        GuiTools::SetCheck(m_hWnd, IDC_FULLINFORADIOBUTTON, true);
    } else {
        GuiTools::SetCheck(m_hWnd, IDC_SUMMARYRADIOBUTTON, true);
        //::EnableWindow(GetDlgItem(IDC_GENERATETEXTINENGLISHCHECKBOX), false);
    }

    GuiTools::SetCheck(m_hWnd, IDC_GENERATETEXTINENGLISHCHECKBOX, generateTextInEnglish_);

    
    ::SetFocus(GetDlgItem(IDC_FILEINFOEDIT));
    FixEditRTL();
    GenerateInfo(); 
    return 0; 
}

LRESULT CMediaInfoDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{    
    if(!IsRunning())  EndDialog(wID); // Don't allow user to close dialog before thread finishes
    return 0;
}

LRESULT CMediaInfoDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(!IsRunning())  EndDialog(wID);
    return 0;
}

void CMediaInfoDlg::ShowInfo(LPCTSTR FileName)
{
    m_FileName = FileName;
    DoModal();
}

DWORD CMediaInfoDlg::Run()
{
    CString  ShortFileName = WinUtils::TrimString(WinUtils::myExtractFileName(m_FileName), 40);
    if(!WinUtils::FileExists(m_FileName))
    { 
        SetDlgItemText(IDC_FILEINFOLABEL, CString(TR("Error:")));
        SetDlgItemText(IDC_FILEINFOEDIT, CString(TR("File \"")) + ShortFileName + TR("\" not found!"));
        return 0;
    }

    SetDlgItemText(IDC_FILEINFOLABEL,CString(TR("Information about file"))+_T(" \"")+ ShortFileName+_T("\" :"));
    
    MediaInfoHelper::GetMediaFileInfo(m_FileName, summary_, fullInfo_, !generateTextInEnglish_);
    bool fullInfo = GuiTools::GetCheck(m_hWnd, IDC_FULLINFORADIOBUTTON);
    SetDlgItemText(IDC_FILEINFOEDIT, fullInfo ? fullInfo_ : summary_);
    return 0;
}

void CMediaInfoDlg::GenerateInfo() {
    if (!IsRunning()) {
        generateTextInEnglish_ = GuiTools::GetCheck(m_hWnd, IDC_GENERATETEXTINENGLISHCHECKBOX);
        Start(); // Starting thread which will load in background information about file m_FileName
    }
}

void CMediaInfoDlg::FixEditRTL() {
    HWND editControl = GetDlgItem(IDC_FILEINFOEDIT);

    if (ServiceLocator::instance()->translator()->isRTL()) {
        LONG styleEx = ::GetWindowLong(editControl, GWL_EXSTYLE);
        LONG style = ::GetWindowLong(editControl, GWL_STYLE);
        //DWORD isStyle = style & ES_RIGHT;

        if (generateTextInEnglish_) {
            styleEx = styleEx & ~(WS_EX_RTLREADING | WS_EX_LAYOUTRTL | WS_EX_RIGHT);
            style = style & (~ES_RIGHT);
        } else {
            styleEx = styleEx | WS_EX_RTLREADING | WS_EX_LAYOUTRTL | WS_EX_RIGHT;
            style = style | ES_RIGHT;
        }
        ::SetWindowLong(editControl, GWL_EXSTYLE, styleEx);
        ::SetWindowLong(editControl, GWL_STYLE, style);
    }
}

LRESULT CMediaInfoDlg::OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // Copying text to clipboard
    SendDlgItemMessage(IDC_FILEINFOEDIT, EM_SETSEL, 0, -1);
    SendDlgItemMessage(IDC_FILEINFOEDIT, WM_COPY, 0, 0);
    SendDlgItemMessage(IDC_FILEINFOEDIT, EM_SETSEL, static_cast<WPARAM>(-1), 0);
    return 0;
}

LRESULT CMediaInfoDlg::OnInfoRadioButtonClicked(WORD, WORD, HWND, BOOL&) {
    bool fullInfo = GuiTools::GetCheck(m_hWnd, IDC_FULLINFORADIOBUTTON);
    //::EnableWindow(GetDlgItem(IDC_GENERATETEXTINENGLISHCHECKBOX), fullInfo);
    SetDlgItemText(IDC_FILEINFOEDIT, fullInfo ? fullInfo_ : summary_);
    Settings.MediaInfoSettings.InfoType = fullInfo ? 1 : 0;
    return 0;
}

LRESULT CMediaInfoDlg::OnShowInEnglishCheckboxClicked(WORD, WORD, HWND, BOOL&) {
    generateTextInEnglish_ = GuiTools::GetCheck(m_hWnd, IDC_GENERATETEXTINENGLISHCHECKBOX);
    
    FixEditRTL();
    Settings.MediaInfoSettings.EnableLocalization = !generateTextInEnglish_;
    GenerateInfo();
    return 0;
}