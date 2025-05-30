#pragma once

#include "atlheaders.h"
#include "resource.h"     
#include "settingspage.h"

// CFFmpegSettingsPage
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
    END_MSG_MAP()
        
    BEGIN_DDX_MAP(CFFmpegSettingsPage)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECCOMBO, videoCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_AUDIOCODECCOMBO, audioCodecComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOCODECPRESETCOMBO, videoCodecPresetComboBox_)
        DDX_CONTROL_HANDLE(IDC_VIDEOBITRATEEDIT, videoBitrateEditControl_)
        DDX_CONTROL_HANDLE(IDC_AUDIOBITRATECOMBO, audioBitrateComboBox_)
    END_DDX_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    bool apply() override;
    void TranslateUI();

    CEdit videoBitrateEditControl_;
    CComboBox videoCodecComboBox_, audioCodecComboBox_, videoCodecPresetComboBox_, audioBitrateComboBox_;
    WtlGuiSettings* settings_;
};

