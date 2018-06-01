/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "UpdateDlg.h"

#include "Func/common.h"
#include "Gui/Dialogs/WizardDlg.h"
#include "Func/CmdLine.h"
#include "Core/Settings.h"
#include "Func/WinUtils.h"

// CUpdateDlg

/* This function doesn't work as intended */
bool CanWriteToFolder(const CString& folder)
{
    HANDLE hFile = ::CreateFile(folder, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    else
    {
        ::CloseHandle(hFile);
        return true;
    }
}

CUpdateDlg::CUpdateDlg()
{
    m_UpdateCallback = NULL;
    m_Checked = false;
    m_bUpdateFinished = false;

    m_Modal = false;
}

CUpdateDlg::~CUpdateDlg()
{
    m_hWnd = 0;
    m_InteractiveUpdate = false;
    m_bClose = false;
}

LRESULT CUpdateDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // FIXME
    DlgResize_Init();
    m_UpdateEvent.Create();
    SetWindowText(TR("Image Uploader Updates"));
    m_listView.AttachToDlgItem(m_hWnd, IDC_UPDATELISTVIEW);
    m_listView.AddColumn(TR("Component name"), 0);
    m_listView.AddColumn( TR("Status"), 1);

    m_listView.SetColumnWidth(0, 170);
    m_listView.SetColumnWidth(1, 290);
    m_UpdateManager.setUpdateStatusCallback(this);

    ::ShowWindow(GetDlgItem(IDOK), SW_HIDE);

    TRC(IDCANCEL, "Cancel");
   
    if (!m_Modal)
        Start();  // Beginning update process
    return 1;  // Let the system set the focus
}

LRESULT CUpdateDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_bUpdateFinished)
    {
        // Begin Update Process
        CString pid = WinUtils::IntToStr(GetCurrentProcessId());

        BOOL elev = false;
        bool isVista = WinUtils::IsVista();
        if (isVista)
            IsElevated(&elev);
        bool CanWrite = WinUtils::IsDirectory(WinUtils::GetAppFolder() + _T("Data"));

        bool NeedElevation = m_UpdateManager.AreCoreUpdates() && isVista && !elev && !CmdLine.IsOption(_T("update"));
        NeedElevation |= isVista && !elev && !CanWrite;
        //    && !CanWriteToFolder(IU_GetDataFolder());
        if (NeedElevation)
        {
            IU_RunElevated(CString(_T("/update ")) + _T("/waitforpid=") + pid);
            m_bClose = 2;

            return 0;
        }
        Start();
    }
    else
    {
        // Closing and reexecuting image uploader
        CString pid = WinUtils::IntToStr(GetCurrentProcessId());
        if (!CmdLine.IsOption(_T("update")))
            IULaunchCopy(_T("/afterupdate /waitforpid=") + pid);  // executing new IU copy with the same command line params
        else
            IULaunchCopy(_T("/afterupdate /waitforpid=") + pid, CAtlArray<CString>());

        m_bClose = 2;
        return 0;
    }

    return 0;
}

LRESULT CUpdateDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (IsRunning())
    {
        m_UpdateManager.stop();
    }
    else
    {
        m_bClose = true; // Closing "modal" dialog
    }
    return 0;
}

void CUpdateDlg::CheckUpdates()
{
    m_listView.ShowWindow(SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPDATEINFO), SW_SHOW);
    m_Checked = true;

    SetDlgItemText(IDC_UPDATEINFO, TR("Checking for updates..."));
    if (!m_UpdateManager.CheckUpdates())
    {
        TRC(IDCANCEL, "Close");
        m_Checked = false;
        CString errorStr = TR("An error occured while receiving update information from server.");
        errorStr += "\r\n";
        errorStr += m_UpdateManager.ErrorString();
        SetDlgItemText(IDC_UPDATEINFO, errorStr);
        return;
    }

    Settings.LastUpdateTime = static_cast<int>(time(0));
    if (m_UpdateManager.AreUpdatesAvailable())
    {
        if ( !m_UpdateManager.AreCoreUpdates() && !IsWindowVisible() ) {
            DoUpdates();
            return;
        }
        if (CmdLine.IsOption(_T("update")))
        {
            DoUpdates();
            return;
        }

        ::ShowWindow(GetDlgItem(IDOK), SW_SHOW);
        TRC(IDOK, "Update");
        if (m_UpdateCallback)
            m_UpdateCallback->UpdateAvailabilityChanged(true);

        if (ShouldStop())
            return;

        if (!IsWindowVisible())
            SetTimer(2, 2000, 0);

        CString text = m_UpdateManager.generateReport();
        SetDlgItemText(IDC_UPDATEINFO, text);
        return;
    }
    else
    {
        TRC(IDCANCEL, "Close");
        CString text = TR("There are no updates available.");
        //text += _T("\r\n\r\n") + m_UpdateManager.generateReportNoUpdates();
        SetDlgItemText(IDC_UPDATEINFO, text);
    }
}

void CUpdateDlg::DoUpdates()
{
    ::ShowWindow(GetDlgItem(IDOK), SW_HIDE);
    ::ShowWindow(GetDlgItem(IDC_UPDATEINFO), SW_HIDE);
    m_listView.DeleteAllItems();

    for (size_t i = 0; i < m_UpdateManager.m_updateList.size(); i++)
    {
        m_listView.AddItem(i, 0, m_UpdateManager.m_updateList[i].displayName());
        m_listView.AddItem(i, 1, TR("Queued"));
    }
    m_listView.ShowWindow(SW_SHOW);
    m_UpdateManager.DoUpdates();

    if (m_UpdateManager.successPackageUpdatesCount())
    {
        ::ShowWindow(GetDlgItem(IDCANCEL), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDOK), SW_SHOW);
        TRC(IDOK, "Finish");
        m_bUpdateFinished = true;
    }
    else
    {
        ::ShowWindow(GetDlgItem(IDCANCEL), SW_SHOW);
        ::ShowWindow(GetDlgItem(IDOK), SW_HIDE);
        TRC(IDCANCEL, "Close");
        m_bUpdateFinished = false;
    }
    m_Checked = false;
}

DWORD CUpdateDlg::Run()
{
    if (!m_Checked)
        CheckUpdates();
    else
        DoUpdates();
    return 0;
}

void CUpdateDlg::updateStatus(int packageIndex, const CString& status)
{
    m_listView.SetItemText(packageIndex, 1, status);
}

bool CUpdateDlg::ShowModal(HWND parent, bool forceCheck)
{
    if (forceCheck) {
        m_Checked = false;
    }
    m_Modal = true;
    Settings.LastUpdateTime = static_cast<int>(time(0));
    if (!m_hWnd)
        Create(parent);
    m_bClose = false;

    KillTimer(2);
    CenterWindow(GetParent());
    ShowWindow(SW_SHOW);
    ::EnableWindow(GetParent(), false);

    if (!m_Checked  && !IsRunning())
    {
        Start();
    }

    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0) && !m_bClose)
    {
        if (!IsDialogMessage(&msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
    if (msg.message == WM_QUIT)
        PostQuitMessage(0);

    m_Modal = false;
    if (m_bClose == 2)
        ::PostMessage(GetParent(), WM_MY_EXIT, 5, 0);

    ::EnableWindow(GetParent(), true);
    ShowWindow(SW_HIDE);
    return true;
}

LRESULT CUpdateDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT wTimerID = wParam;

    if (wTimerID == 2)
    {
        //HWND parent = GetParent();
        if ((m_UpdateCallback && m_UpdateCallback->CanShowWindow()) || !m_UpdateCallback)
        {
            ShowModal(m_hWnd);
            KillTimer(2);
        } else if (m_UpdateCallback){
            m_UpdateCallback->ShowUpdateMessage(m_UpdateManager.generateUpdateMessage());
        }
    }
    else if (wTimerID == 1)
    {
    }
    return 0;
}

void CUpdateDlg::Abort()
{
    if (IsRunning())
    {
        m_UpdateManager.stop();
        Terminate();
    }
}

void CUpdateDlg::setUpdateCallback(CUpdateDlgCallback* callback)
{
    m_UpdateCallback = callback;
}
