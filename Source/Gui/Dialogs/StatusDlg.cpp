/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "StatusDlg.h"

#include "atlheaders.h"
#include "Core/i18n/Translator.h"
#include "Gui/GuiTools.h"
#include "Core/Utils/CoreUtils.h"

CStatusDlg::CStatusDlg(std::shared_ptr<BackgroundTask> task):
    canBeStopped_(true),
	task_(std::move(task)) {

    init();

    finishFuture_ = finishPromise_.get_future();

    taskFinishedConnection_ = task_->onTaskFinished.connect([this](BackgroundTask*, BackgroundTaskResult taskResult) {
        finishPromise_.set_value(taskResult);

        ServiceLocator::instance()->taskRunner()->runInGuiThread([this, taskResult] {
            ProcessFinished();
            if (!IsWindow()) {
                return;
            }
            EndDialog(taskResult == BackgroundTaskResult::Success ? IDOK : IDCANCEL);
        });
    });

    taskProgressConnection_ = task_->onProgress.connect([&](BackgroundTask*, int pos, int max, const std::string& status) {
        CString statusW = U2W(status);
        
        if (!IsWindow()) {
            SetInfo(statusW, _T(""));
            return;
        }
       
        ServiceLocator::instance()->taskRunner()->runInGuiThread([=] {
            updateTitle(statusW);

            if (pos < 0) {
                if ((progressBar_.GetStyle() & PBS_MARQUEE) == 0) {
                    progressBar_.SetWindowLong(GWL_STYLE, progressBar_.GetStyle() | PBS_MARQUEE);
                }
                progressBar_.SetMarquee(TRUE);
            }
            else {
                if ((progressBar_.GetStyle() & PBS_MARQUEE) == PBS_MARQUEE) {
                    progressBar_.SetWindowLong(GWL_STYLE, progressBar_.GetStyle() & ~PBS_MARQUEE);
                    progressBar_.SetMarquee(FALSE);
                }

                progressBar_.SetRange(0, max);
                progressBar_.SetPos(pos);
            }
        });
    });
}

CStatusDlg::CStatusDlg(bool canBeStopped):  
    canBeStopped_(canBeStopped)
{
    init();
}

void CStatusDlg::init() {

}

CStatusDlg::~CStatusDlg()
{
}

LRESULT CStatusDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    progressBar_ = GetDlgItem(IDC_PROGRESSBAR);
    needStop_ = false;

    SetWindowTitle(TR("Process Status"));

    updateTitle(title_);
    updateText(text_);

    SetTimer(kUpdateTimer, 500);
    TRC(IDCANCEL, "Stop");
    ::ShowWindow(GetDlgItem(IDCANCEL), canBeStopped_ ? SW_SHOW : SW_HIDE);
    titleFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_TITLE));
	
    progressBar_.ShowWindow(SW_SHOW);
    progressBar_.SetMarquee(TRUE);
	
	/*if (task_) {
        ServiceLocator::instance()->taskDispatcher()->postTask(task_);
	}*/
    return 1;  // Let the system set the focus
}

LRESULT CStatusDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    needStop_ = true;

	if (task_) {
        task_->cancel();
	}
    KillTimer(kUpdateTimer);
    
    return 0;
}

void CStatusDlg::SetInfo(const CString& title, const CString& text)
{
    title_ = title;
    text_ = text;
    updateTitle(title_);
    updateText(text_);
}

LRESULT CStatusDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (!IsWindowVisible() && !needStop_) {
        ShowWindow(SW_SHOW);
    }

    updateTitle(title_);
    updateText(text_);

    if (processFinished_) {
        EndDialog(IDOK);
    }
    return 0;
}

void CStatusDlg::SetWindowTitle(const CString& WindowTitle)
{
    SetWindowText(WindowTitle);
}

bool CStatusDlg::NeedStop() const
{
    return needStop_;
}

void CStatusDlg::Hide()
{
    KillTimer(kUpdateTimer);
    ShowWindow(SW_HIDE);
    needStop_ = false;
}

int CStatusDlg::executeTask(HWND parent, int timeoutMs) {
    if (!task_) {
        return IDCANCEL;
    }

    try {
        ServiceLocator::instance()->taskDispatcher()->postTask(task_);
    } catch (const std::exception& e) {
        LOG(ERROR) << "Task execution failed: " << e.what();
        return IDCANCEL;
    }

    try {
        if (finishFuture_.wait_for(std::chrono::milliseconds(timeoutMs)) == std::future_status::ready) {
            // Task completed quickly - no dialog needed
            auto result = finishFuture_.get();
            return IDOK;
        } 
    } catch (const std::exception& e) {
        LOG(ERROR) << "Wait on task failed: " << e.what();
    }
    
    // Task is taking long - show dialog
    return DoModal(parent);
}

void CStatusDlg::updateTitle(const std::string& title) {
    if (!IsWindow() || actualTitleUtf8_ == title) {
        return;
    }
    actualTitle_ = U2W(title);
    actualTitleUtf8_ = title;
    SetDlgItemText(IDC_TITLE, title_);
}

void CStatusDlg::updateTitle(const CString& title) {
    if (!IsWindow() || actualTitle_ == title) {
        return;
    }
    actualTitle_ = title;
    actualTitleUtf8_ = W2U(title);
    SetDlgItemText(IDC_TITLE, title_);
}

void CStatusDlg::updateText(const CString& text) {
    if (!IsWindow() || actualText_ == text) {
        return;
    }
    actualText_ = text;
    SetDlgItemText(IDC_TEXT, text);
}

void CStatusDlg::ProcessFinished() {
    processFinished_ = true;
}
