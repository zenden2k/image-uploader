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

#include "TextViewDlg.h"

// CTextViewDlg
CTextViewDlg::CTextViewDlg(const CString &text, const CString &title, const CString &info, const CString &question , const CString &okCaption,const CString &cancelCaption)
{
    m_text = text;
    m_okCaption = okCaption;
    m_cancelCaption = cancelCaption;
    m_question = question;
    m_info = info;
    m_title = title;
}

CTextViewDlg::~CTextViewDlg()
{
}

LRESULT CTextViewDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DlgResize_Init();
    CenterWindow(GetParent());
    SetDlgItemText(IDOK, m_okCaption);
    SetDlgItemText(IDCANCEL, m_cancelCaption);
    SetDlgItemText(IDC_TEXTEDIT, m_text);
    SetDlgItemText(IDC_QUESTIONLABEL, m_question);
    SetDlgItemText(IDC_TITLETEXT,m_info);
    SetWindowText(m_title);
    return 1;  // Let the system set the focus
}

LRESULT CTextViewDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CTextViewDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CTextViewDlg::OnClickedSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDOK);
    return 0;
}
