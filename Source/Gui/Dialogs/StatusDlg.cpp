/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
CStatusDlg::CStatusDlg(bool canBeStopped) : m_bNeedStop(false), canBeStopped_(canBeStopped)
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
    Section2.Lock();
    m_bNeedStop = true;
    Section2.Unlock();
    KillTimer(kUpdateTimer);
    
    return 0;
}

void CStatusDlg::SetInfo(const CString& Title, const CString& Text)
{
    CriticalSection.Lock();
    m_Title=Title;
    m_Text = Text;
    CriticalSection.Unlock();
}

LRESULT CStatusDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(!IsWindowVisible() && !m_bNeedStop)
        ShowWindow(SW_SHOW);
    SetDlgItemText(IDC_TITLE, m_Title);
    SetDlgItemText(IDC_TEXT, m_Text);
    return 0;
}

void CStatusDlg::SetWindowTitle(const CString& WindowTitle)
{
    CriticalSection.Lock();
    SetWindowText(WindowTitle);
    CriticalSection.Unlock();
}

bool CStatusDlg::NeedStop()
{
    Section2.Lock();
    bool res = m_bNeedStop;
    Section2.Unlock();
    return res;
}

void CStatusDlg::Hide()
{
    KillTimer(kUpdateTimer);
    ShowWindow(SW_HIDE);
    m_bNeedStop = false;
}