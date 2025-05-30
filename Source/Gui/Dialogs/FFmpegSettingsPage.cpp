#include "FFmpegSettingsPage.h"

#include "Core/CommonDefs.h"
#include "WizardDlg.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Gui/Components/MyFileDialog.h"
#include "Gui/Components/NewStyleFolderDialog.h"
#include "Core/Settings/WtlGuiSettings.h"

CFFmpegSettingsPage::CFFmpegSettingsPage() {
    settings_ = ServiceLocator::instance()->settings<WtlGuiSettings>();
}

void CFFmpegSettingsPage::TranslateUI() {
    TRC(IDC_VIDEOCODECLABEL, "Video codec:");
    TRC(IDC_VIDEOQUALITYRADIO, "Quality:");
    TRC(IDC_LOWQUALITYLABEL, "low quality (0)");
    TRC(IDC_HIGHQUALITYLABEL, "high quality (100)");
    TRC(IDC_VIDEOBITRATELABEL, "Bitrate:");
    TRC(IDC_VIDEOCODECPRESETLABEL, "Preset:");
    TRC(IDC_AUDIOCODECLABEL, "Audio codec:");
    TRC(IDC_AUDIOBITRATELABEL, "Bitrate:");
}
    
LRESULT CFFmpegSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    TranslateUI();
    DoDataExchange(FALSE);

    /*int backendComboIndex = -1;
    for (const auto& backendName : settings_->ScreenRecordingBackends) {
        int index = backendCombobox_.AddString(U2WC(backendName));
        if (backendName == settings_->ScreenRecordingSettings.Backend) {
            backendComboIndex = index;
        }
    }
    backendCombobox_.SetCurSel(backendComboIndex);

    outFolderEditControl_.SetWindowText(U2W(settings_->ScreenRecordingSettings.OutDirectory));
    ffmpegPathEditControl_.SetWindowText(U2W(settings_->ScreenRecordingSettings.FFmpegCLIPath));
    */
    return 1;  // Let the system set the focus
}

bool CFFmpegSettingsPage::apply() { 
    if (DoDataExchange(TRUE)) {
        /* int backendComboIndex_ = backendCombobox_.GetCurSel();
        if (backendComboIndex_ >= 0 && backendComboIndex_ < settings_->ScreenRecordingBackends.size()) {
            settings_->ScreenRecordingSettings.Backend = settings_->ScreenRecordingBackends[backendComboIndex_];
        }

        settings_->ScreenRecordingSettings.OutDirectory = W2U(GuiTools::GetWindowText(outFolderEditControl_));
        settings_->ScreenRecordingSettings.FFmpegCLIPath = W2U(GuiTools::GetWindowText(ffmpegPathEditControl_));*/
       
        return true;
    }

    return false;
}
