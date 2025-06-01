#include "FFmpegSettingsPage.h"

#include "Core/CommonDefs.h"
#include "WizardDlg.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "ScreenCapture/ScreenRecorder/FFMpegOptionsManager.h"

CFFmpegSettingsPage::CFFmpegSettingsPage() {
    settings_ = ServiceLocator::instance()->settings<WtlGuiSettings>();
    ffmpegOptionsManager_ = std::make_unique<FFMpegOptionsManager>();
}

void CFFmpegSettingsPage::TranslateUI() {
    TRC(IDC_VIDEOSOURCELABEL, "Video source:");
    TRC(IDC_VIDEOCODECLABEL, "Video codec:");
    TRC(IDC_VIDEOQUALITYRADIO, "Quality:");
    TRC(IDC_LOWQUALITYLABEL, "low quality (0)");
    TRC(IDC_HIGHQUALITYLABEL, "high quality (100)");
    TRC(IDC_VIDEOBITRATELABEL, "Bitrate:");
    TRC(IDC_VIDEOCODECPRESETLABEL, "Preset:");
    TRC(IDC_AUDIOCODECLABEL, "Audio codec:");
    TRC(IDC_AUDIOBITRATELABEL, "Bitrate:");
    TRC(IDC_AUDIOSOURCELABEL, "Audio source:");
}
    
void CFFmpegSettingsPage::updateVideoQualityLabel() {
    int pos = videoQualityTrackBar_.GetPos();
    std::wstring text = str(IuStringUtils::FormatWideNoExcept(TR("%d%%")) % pos);
    videoQualityPercentLabel_.SetWindowText(text.c_str());
}

void CFFmpegSettingsPage::videoRadioChanged() {
    BOOL videoQualitySelected = videoQualityRadio_.GetCheck() == BST_CHECKED;
    BOOL codecUsesQuality = codecInfo_ ? codecInfo_->CanUseQuality : TRUE;
    BOOL codecUsesBitrate = codecInfo_ ? codecInfo_->CanUseBitrate : TRUE;

    BOOL videoQualityEnabled = videoQualitySelected && codecUsesQuality;
    videoQualityTrackBar_.EnableWindow(videoQualityEnabled);
    videoQualityPercentLabel_.EnableWindow(videoQualityEnabled);
    lowQualityLabel_.EnableWindow(videoQualityEnabled);
    highQualityLabel_.EnableWindow(videoQualityEnabled);

    BOOL videoBitrateEnabled = codecUsesBitrate && !videoQualitySelected;
    videoBitrateEditControl_.EnableWindow(videoBitrateEnabled);
    videoBitrateUpDownControl_.EnableWindow(videoBitrateEnabled);
    videoBitrateUnitsLabel_.EnableWindow(videoBitrateEnabled);
}

void CFFmpegSettingsPage::videoCodecChanged(const std::string& currentPresetId) {
    int index = videoCodecComboBox_.GetCurSel();
    if (index < 0 || index >= videoCodecs_.size()) {
        return;
    }

    std::string codecId = videoCodecs_[index].first;
    codecInfo_ = ffmpegOptionsManager_->getVideoCodecInfo(codecId);

    fillVideoCodecPresets(codecId, currentPresetId);

    if (codecInfo_) {
        videoQualityRadio_.EnableWindow(codecInfo_->CanUseQuality);
        videoBitrateRadio_.EnableWindow(codecInfo_->CanUseBitrate);

        if (!codecInfo_->CanUseBitrate) {
            videoBitrateRadio_.SetCheck(BST_UNCHECKED);
            videoQualityRadio_.SetCheck(BST_CHECKED);
            videoRadioChanged();
        } else if (!codecInfo_->CanUseQuality) {
            videoQualityRadio_.SetCheck(BST_UNCHECKED);
            videoBitrateRadio_.SetCheck(BST_CHECKED);

            videoRadioChanged();
        }
    }
}

void CFFmpegSettingsPage::fillVideoCodecPresets(const std::string& codecId, const std::string& currentPresetId) {
    if (codecInfo_) {
        videoCodecPresets_ = codecInfo_->Presets;
        const std::string presetId = currentPresetId.empty() ? codecInfo_->DefaultPresetId : currentPresetId;
       
        videoCodecPresetComboBox_.ResetContent();
        int selectedIndex = -1;
        for (const auto& preset : videoCodecPresets_) {
            int index = videoCodecPresetComboBox_.AddString(U2WC(preset.second));
            if (index >= 0 && presetId == preset.first) {
                selectedIndex = index;
            }
        }
        videoCodecPresetComboBox_.SetCurSel(selectedIndex);
    }
}

LRESULT CFFmpegSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    TranslateUI();
    DoDataExchange(FALSE);
    const auto& recordingSettings = settings_->ScreenRecordingSettings.FFmpegSettings;

    videoSources_ = ffmpegOptionsManager_->getVideoSources();
    int selectedItemIndex = -1;
    for (const auto& source : videoSources_) {
        int index = videoSourceComboBox_.AddString(U2WC(source.second));
        if (source.first == recordingSettings.VideoSourceId) {
            selectedItemIndex = index;
        }
    }
    videoSourceComboBox_.SetCurSel(selectedItemIndex);

    audioSources_ = ffmpegOptionsManager_->getAudioSources();
    audioSources_.insert(audioSources_.begin(), { "", tr("None") });
    selectedItemIndex = -1;
    for (const auto& source : audioSources_) {
        int index = audioSourceCombobox_.AddString(U2WC(source.second));
        if (source.first == recordingSettings.AudioSourceId) {
            selectedItemIndex = index;
        }
    }
    audioSourceCombobox_.SetCurSel(selectedItemIndex);

    videoQualityTrackBar_.SetPos(recordingSettings.VideoQuality);
    updateVideoQualityLabel();

    videoBitrateUpDownControl_.SetBase(100);
    videoBitrateUpDownControl_.SetRange32(100, 100000);
    UDACCEL accel { 0, 1 }; 
    accel.nInc = 100; 
    videoBitrateUpDownControl_.SetAccel(1, &accel);
    videoBitrateUpDownControl_.SetPos32(recordingSettings.VideoBitrate);

    videoQualityRadio_.SetCheck(recordingSettings.UseQuality ? BST_CHECKED : BST_UNCHECKED);
    videoBitrateRadio_.SetCheck(recordingSettings.UseQuality ? BST_UNCHECKED : BST_CHECKED);

    videoRadioChanged();

    videoCodecs_ = ffmpegOptionsManager_->getVideoCodecs();
    selectedItemIndex = -1;
    for (const auto& codec : videoCodecs_) {
        int index = videoCodecComboBox_.AddString(U2WC(codec.second));
        if (codec.first == recordingSettings.VideoCodecId) {
            selectedItemIndex = index;
        }
    }

    videoCodecComboBox_.SetCurSel(selectedItemIndex);
    videoCodecChanged(recordingSettings.VideoPresetId);

    return 1;  // Let the system set the focus
}

LRESULT CFFmpegSettingsPage::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    HWND wnd = reinterpret_cast<HWND>(lParam);
    if (wnd == videoQualityTrackBar_) {
        updateVideoQualityLabel();
    }
    return 0;
}

LRESULT CFFmpegSettingsPage::OnVideoRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    videoRadioChanged();
    return 0;
}

LRESULT CFFmpegSettingsPage::OnVideoCodecChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
    videoCodecChanged();
    return 0;
}

bool CFFmpegSettingsPage::apply() { 
    if (DoDataExchange(TRUE)) {
        auto& recordingSettings = settings_->ScreenRecordingSettings.FFmpegSettings;

        recordingSettings.VideoQuality = videoQualityTrackBar_.GetPos();
        recordingSettings.VideoBitrate = videoBitrateUpDownControl_.GetPos32();
        recordingSettings.UseQuality = videoQualityRadio_.GetCheck() == BST_CHECKED;

        int videoCodecIndex = videoCodecComboBox_.GetCurSel();
        if (videoCodecIndex >= 0 && videoCodecIndex < videoCodecs_.size()) {
            recordingSettings.VideoCodecId = videoCodecs_[videoCodecIndex].first;
        }

        int videoPresetIndex = videoCodecPresetComboBox_.GetCurSel();
        if (videoPresetIndex >= 0 && videoPresetIndex < videoCodecPresets_.size()) {
            recordingSettings.VideoPresetId = videoCodecPresets_[videoPresetIndex].first;
        }

        int videoSourceIndex = videoSourceComboBox_.GetCurSel();
        if (videoSourceIndex >= 0 && videoSourceIndex < videoSources_.size()) {
            recordingSettings.VideoSourceId = videoSources_[videoSourceIndex].first;
        }

        int audioSourceIndex = audioSourceCombobox_.GetCurSel();
        if (audioSourceIndex >= 0 && audioSourceIndex < audioSources_.size()) {
            recordingSettings.AudioSourceId = audioSources_[audioSourceIndex].first;
        }

        return true;
    }

    return false;
}
