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

#include "SearchByImageDlg.h"

#include "atlheaders.h"
#include "Core/i18n/Translator.h"
#include "Gui/GuiTools.h"
#include "Core/SearchByImage.h"
#include "Func/WinUtils.h"
#include "Core/Network/NetworkClientFactory.h"
#include "Core/Settings/CommonGuiSettings.h"
// CSearchByImageDlg

CSearchByImageDlg::CSearchByImageDlg(UploadManager* uploadManager, SearchByImage::SearchEngine searchEngine, CString fileName):
    uploadManager_(uploadManager)
{
    fileName_ = fileName;
    cancelPressed_ = false;
    searchEngine_ = searchEngine;
    finished_ = false;
}

CSearchByImageDlg::~CSearchByImageDlg()
{
}

LRESULT CSearchByImageDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    TRC(IDCANCEL, "Cancel");
    GuiTools::MakeLabelBold(GetDlgItem(IDC_TITLE));
    SetWindowText(TR("Search by image"));

    HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
    if (hWnd) {
        wndAnimation_.SubclassWindow(hWnd);
        //wndAnimation_.ShowWindow(SW_HIDE);
    }
    auto* settings = ServiceLocator::instance()->settings<CommonGuiSettings>();

    using namespace std::placeholders;
    seeker_ = SearchByImage::createSearchEngine(ServiceLocator::instance()->networkClientFactory(), uploadManager_, searchEngine_, settings->temporaryServer.getByIndex(0), W2U(fileName_));
    seeker_->onTaskFinished.connect([this](BackgroundTask* task, BackgroundTaskResult result) {
        onSeekerFinished(result == BackgroundTaskResult::Success, dynamic_cast<SearchByImageTask*>(task)->message());
    });
    SetDlgItemText(IDC_TEXT, TR("Uploading image..."));
    ServiceLocator::instance()->taskDispatcher()->postTask(seeker_);
    return 1;  // Let the system set the focus
}

LRESULT CSearchByImageDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    cancelPressed_ = true;
    seeker_->cancel();

    if (finished_) {
        EndDialog(IDCANCEL);
    }
    return 0;
}


void CSearchByImageDlg::onSeekerFinished(bool success, const std::string& msg) {
    finished_ = true;
    wndAnimation_.ShowWindow(SW_HIDE);
    if (success) {
        EndDialog(IDOK);
    } else {
        SetDlgItemText(IDC_TEXT, Utf8ToWCstring(msg));
        if (cancelPressed_) {
            EndDialog(IDCANCEL);
        }
    }
}