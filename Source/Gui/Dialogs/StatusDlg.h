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
#ifndef STATUSDLG_H
#define STATUSDLG_H

#pragma once
#include <atomic>
#include <mutex>
#include <memory>
#include <future>

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/BackgroundTask.h"
#include "Gui/Controls/DialogIndirect.h"

// CStatusDlg

class CStatusDlg : public CCustomDialogIndirectImpl<CStatusDlg>, public std::enable_shared_from_this<CStatusDlg> {
private:
    struct PrivateToken { };
public:
    CStatusDlg(PrivateToken, bool canBeStopped = true);
    CStatusDlg(PrivateToken, std::shared_ptr<BackgroundTask> task);
        ~CStatusDlg();
        enum { IDD = IDD_STATUSDLG };
        enum { kUpdateTimer = 1};

        BEGIN_MSG_MAP(CStatusDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_TIMER, OnTimer)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        END_MSG_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        void SetInfo(const CString& Title, const CString& Text);
        void SetWindowTitle(const CString& WindowTitle);

        bool NeedStop() const;
        void ProcessFinished();
        void Hide();

        int executeTask(HWND parent, int timeoutMs = 300);

        static std::shared_ptr<CStatusDlg> create(bool canBeStopped = true);
        static std::shared_ptr<CStatusDlg> create(std::shared_ptr<BackgroundTask> task);


    private:
        void init();
        void updateTitle(const std::string& title);
        void updateTitle(const CString& title);
        void updateText(const CString& title);

        CString title_, text_;
        CString actualTitle_, actualText_;
        std::string actualTitleUtf8_;
        std::atomic_bool needStop_ = false;
        CFont titleFont_;
        bool canBeStopped_;
        bool processFinished_ = false;
        std::mutex mutex_;
        std::promise<BackgroundTaskResult> finishPromise_;
        std::future<BackgroundTaskResult> finishFuture_;
        std::shared_ptr<BackgroundTask> task_;
        CProgressBarCtrl progressBar_;
        boost::signals2::scoped_connection taskFinishedConnection_, taskProgressConnection_;
};

#endif // STATUSDLG_H
