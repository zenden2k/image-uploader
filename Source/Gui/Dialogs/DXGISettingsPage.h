#pragma once

#include <vector>
#include <string>
#include <map>

#include "atlheaders.h"
#include "resource.h"     
#include "settingspage.h"
#include "ScreenCapture/ScreenRecorder/DXGIOptionsManager.h"

class WtlGuiSettings;

class CDXGISettingsPage : 
    public CDialogImpl<CDXGISettingsPage>,
    public CSettingsPage,
    public CWinDataExchange<CDXGISettingsPage>
{
public:
    CDXGISettingsPage();
    virtual ~CDXGISettingsPage() = default;
    enum { IDD = IDD_DXGISETTINGS };

    BEGIN_MSG_MAP(CDXGISettingsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        COMMAND_HANDLER(IDC_VIDEOQUALITYRADIO, BN_CLICKED, OnVideoRadioClicked)
        COMMAND_HANDLER(IDC_VIDEOBITRATELABEL, BN_CLICKED, OnVideoRadioClicked)
        COMMAND_HANDLER(IDC_VIDEOCODECCOMBO, CBN_SELCHANGE, OnVideoCodecChanged)
        COMMAND_HANDLER(IDC_AUDIOCODECCOMBO, CBN_SELCHANGE, OnAudioCodecChanged)
        END_MSG_MAP()
        
    BEGIN_DDX_MAP(CDXGISettingsPage)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECCOMBO, videoCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECPRESETCOMBO, videoCodecPresetComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEEDIT, videoBitrateEditControl_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYSLIDER, videoQualityTrackBar_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYPERCENTLABEL, videoQualityPercentLabel_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATESPIN, videoBitrateUpDownControl_)
        DDX_CONTROL_HANDLE(IDC_VIDEOQUALITYRADIO, videoQualityRadio_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATERADIO, videoBitrateRadio_)
        DDX_CONTROL_HANDLE(IDC_LOWQUALITYLABEL, lowQualityLabel_)
        DDX_CONTROL_HANDLE(IDC_HIGHQUALITYLABEL, highQualityLabel_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEUNITSLABEL, videoBitrateUnitsLabel_)
        DDX_CONTROL_HANDLE(IDC_AUDIOSOURCESLISTVIEW, audioSourcesListView_)
        DDX_CONTROL_HANDLE(IDC_AUDIOCODECCOMBO, audioCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_AUDIOBITRATESPIN, audioBitrateUpDownControl_)
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

    CEdit videoBitrateEditControl_;
    CComboBox videoCodecComboBox_, videoCodecPresetComboBox_;
    CListViewCtrl audioSourcesListView_;
    CComboBox audioCodecComboBox_;
    CTrackBarCtrl videoQualityTrackBar_;
    CStatic videoQualityPercentLabel_, lowQualityLabel_, highQualityLabel_, videoBitrateUnitsLabel_;
    CUpDownCtrl videoBitrateUpDownControl_, audioBitrateUpDownControl_;
    CButton videoQualityRadio_, videoBitrateRadio_;

private:
    WtlGuiSettings* settings_;
    std::unique_ptr<DXGIOptionsManager> optionsManager_;

    // first = CodecId, second = CodecName
    IdNameArray videoCodecs_, videoSources_, audioSources_, audioCodecs_;

    std::string currentVideoCodecId_;
    std::optional<DXGIOptionsManager::VideoCodecInfo> codecInfo_;
    std::optional<DXGIOptionsManager::AudioCodecInfo> audioCodecInfo_;
    bool apply() override;
    void TranslateUI();
    void updateVideoQualityLabel();
    void videoRadioChanged();
    void videoCodecChanged(const std::string& currentPresetId = {});
    void audioCodecChanged(int bitrate = 0, const std::string& currentPresetId = {});
    void fillVideoCodecPresets(const std::string& codecId, const std::string& currentPresetId);
};

