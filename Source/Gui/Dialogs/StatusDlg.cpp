/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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

CStatusDlg::CStatusDlg(PrivateToken, std::shared_ptr<BackgroundTask> task)
    :
    canBeStopped_(true),
	task_(std::move(task)) {
}

CStatusDlg::CStatusDlg(PrivateToken, bool canBeStopped)
    :
    canBeStopped_(canBeStopped)
{
}

void CStatusDlg::init() {
    if (task_) {
        try {
            finishFuture_ = finishPromise_.get_future();
        } catch (const std::exception& ex) {
            LOG(ERROR) << ex.what();
        }

        taskFinishedConnection_ = task_->onTaskFinished.connect([pthis = shared_from_this()](BackgroundTask*, BackgroundTaskResult taskResult) {
            try {
                pthis->finishPromise_.set_value(taskResult);
            } catch (const std::exception& ex) {
                LOG(ERROR) << ex.what();
            }

            pthis->ProcessFinished();
            ServiceLocator::instance()->taskRunner()->runInGuiThread([pthis, taskResult] {
                pthis->ProcessFinished();
                if (!pthis->IsWindow()) {
                    return;
                }

                pthis->EndDialog(taskResult == BackgroundTaskResult::Success ? IDOK : IDCANCEL);
            });
        });

        taskProgressConnection_ = task_->onProgress.connect([pthis = shared_from_this()](BackgroundTask*, int pos, int max, const std::string& status) {
            CString statusW = U2W(status);

            if (!pthis->IsWindow()) {
                pthis->SetInfo(statusW, _T(""));
                return;
            }

            ServiceLocator::instance()->taskRunner()->runInGuiThread([pthis, pos, max, statusW] {
                pthis->updateTitle(statusW);

                if (!pthis->IsWindow()) {
                    return;
                }
                if (pos < 0) {
                    if ((pthis->progressBar_.GetStyle() & PBS_MARQUEE) == 0) {
                        pthis->progressBar_.SetWindowLong(GWL_STYLE, pthis->progressBar_.GetStyle() | PBS_MARQUEE);
                    }
                    pthis->progressBar_.SetMarquee(TRUE);
                } else {
                    if ((pthis->progressBar_.GetStyle() & PBS_MARQUEE) == PBS_MARQUEE) {
                        pthis->progressBar_.SetWindowLong(GWL_STYLE, pthis->progressBar_.GetStyle() & ~PBS_MARQUEE);
                        pthis->progressBar_.SetMarquee(FALSE);
                    }

                    pthis->progressBar_.SetRange(0, max);
                    pthis->progressBar_.SetPos(pos);
                }
            });
        });
    }
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

std::shared_ptr<CStatusDlg> CStatusDlg::create(std::shared_ptr<BackgroundTask> task) {
    auto result = std::make_shared<CStatusDlg>(PrivateToken{}, std::move(task));
    result->init();
    return result;
}

std::shared_ptr<CStatusDlg> CStatusDlg::create(bool canBeStopped /*= true*/) {
    auto result = std::make_shared<CStatusDlg>(PrivateToken{}, canBeStopped);
    result->init();
    return result;
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
