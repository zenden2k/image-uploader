#pragma once

#include <vector>
#include <string>
#include <map>

#include "atlheaders.h"
#include "resource.h"     
#include "settingspage.h"
#include "ScreenCapture/ScreenRecorder/FFMpegOptionsManager.h"

class WtlGuiSettings;

class CFFmpegSettingsPage : 
    public CDialogImpl<CFFmpegSettingsPage>,
    public CSettingsPage,
    public CWinDataExchange<CFFmpegSettingsPage>
{
public:
    CFFmpegSettingsPage();
    virtual ~CFFmpegSettingsPage() = default;
    enum { IDD = IDD_FFMPEGSETTINGS };

    BEGIN_MSG_MAP(CFFmpegSettingsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        COMMAND_HANDLER(IDC_FFMPEGPATHBROWSEBUTTON, BN_CLICKED, OnBnClickedFFmpegBrowseButton)
        COMMAND_HANDLER(IDC_VIDEOQUALITYRADIO, BN_CLICKED, OnVideoRadioClicked)
        COMMAND_HANDLER(IDC_VIDEOBITRATELABEL, BN_CLICKED, OnVideoRadioClicked)
        COMMAND_HANDLER(IDC_VIDEOCODECCOMBO, CBN_SELCHANGE, OnVideoCodecChanged)
        COMMAND_HANDLER(IDC_AUDIOCODECCOMBO, CBN_SELCHANGE, OnAudioCodecChanged)
    END_MSG_MAP()
        
    BEGIN_DDX_MAP(CFFmpegSettingsPage)
        DDX_CONTROL_HANDLE(IDC_FFMPEGPATHEDIT, ffmpegPathEditControl_)
        DDX_CONTROL_HANDLE(IDC_VIDEOSOURCECOMBO, videoSourceComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECCOMBO, videoCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_AUDIOCODECCOMBO, audioCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECPRESETCOMBO, videoCodecPresetComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEEDIT, videoBitrateEditControl_)
        DDX_CONTROL_HANDLE(IDC_AUDIOQUALITYCOMBO, audioQualityComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYSLIDER, videoQualityTrackBar_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYPERCENTLABEL, videoQualityPercentLabel_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATESPIN, videoBitrateUpDownControl_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYRADIO, videoQualityRadio_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATERADIO, videoBitrateRadio_)
        DDX_CONTROL_HANDLE(IDC_LOWQUALITYLABEL, lowQualityLabel_)
        DDX_CONTROL_HANDLE(IDC_HIGHQUALITYLABEL, highQualityLabel_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEUNITSLABEL, videoBitrateUnitsLabel_)
        DDX_CONTROL_HANDLE(IDC_AUDIOSOURCECOMBO, audioSourceCombobox_)
    END_DDX_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnVideoRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnVideoCodecChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAudioCodecChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBnClickedFFmpegBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    CEdit ffmpegPathEditControl_, videoBitrateEditControl_;
    CComboBox videoSourceComboBox_, videoCodecComboBox_, videoCodecPresetComboBox_;
    CComboBox audioSourceCombobox_, audioQualityComboBox_, audioCodecComboBox_;
    CTrackBarCtrl videoQualityTrackBar_;
    CStatic videoQualityPercentLabel_, lowQualityLabel_, highQualityLabel_, videoBitrateUnitsLabel_;
    CUpDownCtrl videoBitrateUpDownControl_;
    CButton videoQualityRadio_, videoBitrateRadio_;

private:
    WtlGuiSettings* settings_;
    std::unique_ptr<FFMpegOptionsManager> ffmpegOptionsManager_;

    // first = CodecId, second = CodecName
    IdNameArray videoCodecs_, videoSources_, audioSources_, audioCodecs_;

    std::string currentVideoCodecId_;
    std::optional<FFMpegOptionsManager::VideoCodecInfo> codecInfo_;
    std::optional<FFMpegOptionsManager::AudioCodecInfo> audioCodecInfo_;
    bool apply() override;
    void TranslateUI();
    void updateVideoQualityLabel();
    void videoRadioChanged();
    void videoCodecChanged(const std::string& currentPresetId = {});
    void audioCodecChanged(const std::string& quality = {}, const std::string& currentPresetId = {});
    void fillVideoCodecPresets(const std::string& codecId, const std::string& currentPresetId);
    void fillAudioCodecQualities(const std::string& codecId, const std::string& currentQuality);
};

