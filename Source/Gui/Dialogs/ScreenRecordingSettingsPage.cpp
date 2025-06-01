#include "ScreenRecordingSettingsPage.h"

#include "Core/CommonDefs.h"
#include "WizardDlg.h"
#include "Gui/GuiTools.h"
#include "Func/WinUtils.h"
#include "Gui/Components/MyFileDialog.h"
#include "Gui/Components/NewStyleFolderDialog.h"
#include "Core/Settings/WtlGuiSettings.h"
#include "FFmpegSettingsPage.h"

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

template <typename T, typename... Args>
std::unique_ptr<T> createPageObject(HWND hWnd, RECT& rc, Args&&... args)
{
    auto dlg = std::make_unique<T>(std::forward<Args>(args)...);
    dlg->Create(hWnd, rc);
    dlg->PageWnd = dlg->m_hWnd;
    return dlg;
}
void CScreenRecordingSettingsPage::showSubPage(SubPage pageId) {
    if (pageId < 0 || pageId >= std::size(subPages_)) {
        return;
    }

    if (!subPages_[pageId]) {
        RECT rc = { 150, 3, 636, 400 };
        auto createObject = [&]() -> std::unique_ptr<CSettingsPage> {
            switch (pageId) {
            case spFFmpegSettings:
                return createPageObject<CFFmpegSettingsPage>(m_hWnd, rc);
            default:
                LOG(ERROR) << "No such page " << pageId;
                return {};
            }
        };

        std::unique_ptr<CSettingsPage> page = createObject();
        if (!page) {
            return;
        }
        subPages_[pageId] = std::move(page);

        WINDOWPLACEMENT wp;
        HWND placeholder = GetDlgItem(IDC_SUBPAGEPLACEHOLDER);
        ::GetWindowPlacement(placeholder, &wp);
        ::SetWindowPos(subPages_[pageId]->PageWnd, placeholder, wp.rcNormalPosition.left, wp.rcNormalPosition.top, -wp.rcNormalPosition.left + wp.rcNormalPosition.right, -wp.rcNormalPosition.top + wp.rcNormalPosition.bottom, 0);

        subPages_[pageId]->fixBackground();
    }

    if (!subPages_[pageId]) {
        return;
    }

    HWND wnd = subPages_[pageId]->PageWnd;

    if (wnd) {
        ::ShowWindow(wnd, SW_SHOW);
    }

    if (curPage_ != spNone && subPages_[curPage_]) {
        ::ShowWindow(subPages_[curPage_]->PageWnd, SW_HIDE);
    }
    curPage_ = pageId;
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

    showSubPage(static_cast<SubPage>(backendComboIndex));

    outFolderEditControl_.SetWindowText(U2W(settings_->ScreenRecordingSettings.OutDirectory));
    ffmpegPathEditControl_.SetWindowText(U2W(settings_->ScreenRecordingSettings.FFmpegSettings.FFmpegCLIPath));

    return 1;  // Let the system set the focus
}

bool CScreenRecordingSettingsPage::apply() { 
    if (DoDataExchange(TRUE)) {

        for (int i = 0; i < std::size(subPages_); i++) {
            const auto& page = subPages_[i];
            if (!page) {
                continue;
            }
            page->clearErrors();

            if (!page->validate()) {
                showSubPage(static_cast<SubPage>(i));
                const auto& errors = page->errors();
                if (!errors.empty()) {
                    CString msg;
                    for (const auto& error : errors) {
                        msg += error.Message;
                        msg += _T("\r\n");
                    }
                    GuiTools::LocalizedMessageBox(m_hWnd, msg, TR("Error"), MB_ICONERROR);
                    if (errors[0].Control) {
                        ::SetFocus(errors[0].Control);
                    }
                }
                return false;
            }
        }

        for (int i = 0; i < std::size(subPages_); i++) {
            try {
                if (subPages_[i] && !subPages_[i]->apply()) {
                    showSubPage(static_cast<SubPage>(i));
                    return false;
                }
            } catch (ValidationException& ex) {
                showSubPage(static_cast<SubPage>(i));
                if (!ex.errors_.empty()) {
                    GuiTools::LocalizedMessageBox(m_hWnd, ex.errors_[0].Message, TR("Error"), MB_ICONERROR);
                    if (ex.errors_[0].Control) {
                        ::SetFocus(ex.errors_[0].Control);
                    }
                }

                return false;
                // If some tab cannot apply changes - do not close dialog
            }
        }


        int backendComboIndex_ = backendCombobox_.GetCurSel();
        if (backendComboIndex_ >= 0 && backendComboIndex_ < settings_->ScreenRecordingBackends.size()) {
            settings_->ScreenRecordingSettings.Backend = settings_->ScreenRecordingBackends[backendComboIndex_];
        }

        settings_->ScreenRecordingSettings.OutDirectory = W2U(GuiTools::GetWindowText(outFolderEditControl_));
        settings_->ScreenRecordingSettings.FFmpegSettings.FFmpegCLIPath = W2U(GuiTools::GetWindowText(ffmpegPathEditControl_));
       
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
