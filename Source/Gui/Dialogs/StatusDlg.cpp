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
// CStatusDlg
CStatusDlg::CStatusDlg(bool canBeStopped) : 
    m_bNeedStop(false), 
    canBeStopped_(canBeStopped), 
    processFinished_(false)
{
        
}

CStatusDlg::~CStatusDlg()
{
}

LRESULT CStatusDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CenterWindow(GetParent());
    m_bNeedStop = false;
    SetDlgItemText(IDC_TITLE, m_Title);
    SetDlgItemText(IDC_TEXT, m_Text);
    SetTimer(kUpdateTimer, 500);
    TRC(IDCANCEL, "Stop");
    ::ShowWindow(GetDlgItem(IDCANCEL), canBeStopped_ ? SW_SHOW : SW_HIDE);
    titleFont_ = GuiTools::MakeLabelBold(GetDlgItem(IDC_TITLE));
    return 1;  // Let the system set the focus
}

LRESULT CStatusDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    std::lock_guard<std::mutex> lk(Section2);
    m_bNeedStop = true;
    KillTimer(kUpdateTimer);
    
    return 0;
}

void CStatusDlg::SetInfo(const CString& Title, const CString& Text)
{
    std::lock_guard<std::mutex> lk(CriticalSection);
    m_Title=Title;
    m_Text = Text;
}

LRESULT CStatusDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(!IsWindowVisible() && !m_bNeedStop)
        ShowWindow(SW_SHOW);
    SetDlgItemText(IDC_TITLE, m_Title);
    SetDlgItemText(IDC_TEXT, m_Text);

    if (processFinished_) {
        EndDialog(IDOK);
    }
    return 0;
}

void CStatusDlg::SetWindowTitle(const CString& WindowTitle)
{
    std::lock_guard<std::mutex> lk(CriticalSection);
    SetWindowText(WindowTitle);
}

bool CStatusDlg::NeedStop()
{
    std::lock_guard<std::mutex> lk(Section2);
    bool res = m_bNeedStop;
    return res;
}

void CStatusDlg::Hide()
{
    KillTimer(kUpdateTimer);
    ShowWindow(SW_HIDE);
    m_bNeedStop = false;
}

void CStatusDlg::ProcessFinished() {
    processFinished_ = true;
}