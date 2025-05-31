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
        COMMAND_HANDLER(IDC_VIDEOQUALITYRADIO, BN_CLICKED, OnVideoRadioClicked)
        COMMAND_HANDLER(IDC_VIDEOBITRATELABEL, BN_CLICKED, OnVideoRadioClicked)
        COMMAND_HANDLER(IDC_VIDEOCODECCOMBO, CBN_SELCHANGE, OnVideoCodecChanged)
    END_MSG_MAP()
        
    BEGIN_DDX_MAP(CFFmpegSettingsPage)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECCOMBO, videoCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_AUDIOCODECCOMBO, audioCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECPRESETCOMBO, videoCodecPresetComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEEDIT, videoBitrateEditControl_)
        DDX_CONTROL_HANDLE(IDC_AUDIOBITRATECOMBO, audioBitrateComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYSLIDER, videoQualityTrackBar_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYPERCENTLABEL, videoQualityPercentLabel_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATESPIN, videoBitrateUpDownControl_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYRADIO, videoQualityRadio_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATERADIO, videoBitrateRadio_)
        DDX_CONTROL_HANDLE(IDC_LOWQUALITYLABEL, lowQualityLabel_)
        DDX_CONTROL_HANDLE(IDC_HIGHQUALITYLABEL, highQualityLabel_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEUNITSLABEL, videoBitrateUnitsLabel_)
    END_DDX_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnVideoRadioClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnVideoCodecChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    CEdit videoBitrateEditControl_;
    CComboBox videoCodecComboBox_, audioCodecComboBox_, videoCodecPresetComboBox_, audioBitrateComboBox_;
    CTrackBarCtrl videoQualityTrackBar_;
    CStatic videoQualityPercentLabel_, lowQualityLabel_, highQualityLabel_, videoBitrateUnitsLabel_;
    CUpDownCtrl videoBitrateUpDownControl_;
    CButton videoQualityRadio_, videoBitrateRadio_;

private:
    WtlGuiSettings* settings_;
    std::unique_ptr<FFMpegOptionsManager> ffmpegOptionsManager_;

    // first = CodecId, second = CodecName
    std::vector<std::pair<std::string, std::string>> videoCodecs_, videoCodecPresets_;
    std::string currentVideoCodecId_;
    std::optional<FFMpegOptionsManager::VideoCodecInfo> codecInfo_;
    bool apply() override;
    void TranslateUI();
    void updateVideoQualityLabel();
    void videoRadioChanged();
    void videoCodecChanged(const std::string& currentPresetId = {});
    void fillVideoCodecPresets(const std::string& codecId, const std::string& currentPresetId);
};

