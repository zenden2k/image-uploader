#include "ScreenRecordingSettingsPage.h"

#include "Core/CommonDefs.h"
#include "WizardDlg.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Gui/Components/MyFileDialog.h"
#include "Gui/Components/NewStyleFolderDialog.h"
#include "Core/Settings/WtlGuiSettings.h"

CScreenRecordingSettingsPage::CScreenRecordingSettingsPage() {
    settings_ = ServiceLocator::instance()->settings<WtlGuiSettings>();
}

void CScreenRecordingSettingsPage::TranslateUI() {
    TRC(IDC_BACKENDLABEL, "Backend:");
    TRC(IDC_OUTFOLDERLABEL, "Video recordings folder:");
    TRC(IDC_OUTFOLDERBROWSEBUTTON, "Browse...");
    TRC(IDC_FFMPEGPATHLABEL, "FFmpeg executable path:");
    TRC(IDC_FFMPEGPATHBROWSEBUTTON, "Browse...");
}
    
LRESULT CScreenRecordingSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    TranslateUI();
    DoDataExchange(FALSE);

    int backendComboIndex = -1;
    for (const auto& backendName : settings_->ScreenRecordingBackends) {
        int index = backendCombobox_.AddString(U2WC(backendName));
        if (backendName == settings_->ScreenRecordingSettings.Backend) {
            backendComboIndex = index;
        }
    }
    backendCombobox_.SetCurSel(backendComboIndex);

    outFolderEditControl_.SetWindowText(U2W(settings_->ScreenRecordingSettings.OutDirectory));
    ffmpegPathEditControl_.SetWindowText(U2W(settings_->ScreenRecordingSettings.FFmpegCLIPath));

    return 1;  // Let the system set the focus
}

bool CScreenRecordingSettingsPage::apply() { 
    if (DoDataExchange(TRUE)) {
        int backendComboIndex_ = backendCombobox_.GetCurSel();
        if (backendComboIndex_ >= 0 && backendComboIndex_ < settings_->ScreenRecordingBackends.size()) {
            settings_->ScreenRecordingSettings.Backend = settings_->ScreenRecordingBackends[backendComboIndex_];
        }

        settings_->ScreenRecordingSettings.OutDirectory = W2U(GuiTools::GetWindowText(outFolderEditControl_));
        settings_->ScreenRecordingSettings.FFmpegCLIPath = W2U(GuiTools::GetWindowText(ffmpegPathEditControl_));
       
        return true;
    }

    return false;
}

LRESULT CScreenRecordingSettingsPage::OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    HWND editControl = GetDlgItem(IDC_SCREENSHOTFOLDEREDIT);
    CString folder;
    outFolderEditControl_.GetWindowText(folder);
    CNewStyleFolderDialog fd(m_hWnd, folder, TR("Select folder"));

    fd.SetFolder(folder);

    if (fd.DoModal(m_hWnd) == IDOK) {
        outFolderEditControl_.SetWindowText(fd.GetFolderPath());
        return true;
    }
    
    return 0;
}

LRESULT CScreenRecordingSettingsPage::OnBnClickedFFmpegBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
    IMyFileDialog::FileFilterArray filters = {
        {
            CString(TR("Executables")),
            _T("*.exe;*.com;*.bat;*.cmd;"),
        },
        { TR("All files"), _T("*.*") }
    };

    auto dlg = MyFileDialogFactory::createFileDialog(m_hWnd, WinUtils::GetAppFolder(), CString(), filters, false);
    CString fileName = GuiTools::GetWindowText(ffmpegPathEditControl_);

    if (fileName.IsEmpty()) {
        fileName = _T("ffmpeg.exe");
    }

    dlg->setFileName(fileName);


    if (dlg->DoModal(m_hWnd) == IDOK) {
        ffmpegPathEditControl_.SetWindowText(dlg->getFile());
        return 0;
    }
    return 0;
}
