#pragma once

#include "atlheaders.h"
#include "resource.h"     
#include "settingspage.h"

// CScreenRecordingSettingsPage
class WtlGuiSettings;

class CScreenRecordingSettingsPage : 
    public CDialogImpl<CScreenRecordingSettingsPage>,
    public CSettingsPage,
    public CWinDataExchange<CScreenRecordingSettingsPage>
{
public:
    CScreenRecordingSettingsPage();
    virtual ~CScreenRecordingSettingsPage() = default;
    enum { IDD = IDD_SCREENRECORDINGSETTINGSPAGE };

    BEGIN_MSG_MAP(CScreenRecordingSettingsPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_OUTFOLDERBROWSEBUTTON, BN_CLICKED, OnBnClickedBrowseButton)
        COMMAND_HANDLER(IDC_FFMPEGPATHBROWSEBUTTON, BN_CLICKED, OnBnClickedFFmpegBrowseButton)
    END_MSG_MAP()
        
    BEGIN_DDX_MAP(CScreenRecordingSettingsPage)
        DDX_CONTROL_HANDLE(IDC_BACKENDCOMBO, backendCombobox_)
        DDX_CONTROL_HANDLE(IDC_OUTFOLDEREDIT, outFolderEditControl_)
        DDX_CONTROL_HANDLE(IDC_FFMPEGPATHEDIT, ffmpegPathEditControl_)
    END_DDX_MAP()

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    bool apply() override;
    void TranslateUI();
    LRESULT OnBnClickedBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnBnClickedFFmpegBrowseButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    CEdit outFolderEditControl_, ffmpegPathEditControl_;
    CComboBox backendCombobox_;
    WtlGuiSettings* settings_;
};

