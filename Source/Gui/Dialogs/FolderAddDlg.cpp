#include "FolderAddDlg.h"

#include "Func/WinUtils.h"
#include "Core/ServiceLocator.h"
#include "WizardDlg.h"
#include "MainDlg.h"
#include <io.h>
#include "Func/IuCommonFunctions.h"

CFolderAdd::CFolderAdd(CWizardDlg *WizardDlg) : m_pWizardDlg(WizardDlg)
{
    m_bSubDirs = true;
}

void CFolderAdd::Do(CStringList &Paths, bool ImagesOnly, bool SubDirs)
{
    count = 0;
    m_bImagesOnly = ImagesOnly;
    RECT rc = { 0, 0, 0, 0 };
    m_bSubDirs = SubDirs;
    if (!dlg.m_hWnd) {
        dlg.Create(m_pWizardDlg->m_hWnd, rc);
    }
    dlg.SetWindowTitle(CString(ImagesOnly ? TR("Searching for picture files...") : TR("Collecting files...")));
    m_Paths.RemoveAll();
    m_Paths.Copy(Paths);
    findfile = 0;
    ZeroMemory(&wfd, sizeof(wfd));

    Start(THREAD_PRIORITY_BELOW_NORMAL);
}

int CFolderAdd::ProcessDir(CString currentDir, bool bRecursive /* = true  */)
{
    CString strWildcard;

    dlg.SetInfo(CString(TR("Processing folder:")), currentDir);
    strWildcard = currentDir + "\\*";

    _tfinddata_t s_Dir;
    intptr_t hDir;

    if ((hDir = _tfindfirst(strWildcard, &s_Dir)) == -1L)

        return 1;

    do {
        if (dlg.NeedStop()) { _findclose(hDir); return 0; }

        if (s_Dir.name[0] != '.' && (s_Dir.attrib & _A_SUBDIR) && bRecursive == true) {
            ProcessDir(currentDir + '\\' + s_Dir.name, bRecursive);
        } else if (s_Dir.name[0] != '.') {
            if (!m_bImagesOnly || IuCommonFunctions::IsImage(s_Dir.name)) {
                CWizardDlg::AddImageStruct ais;
                ais.show = !m_pWizardDlg->QuickUploadMarker;
                CString name = CString(currentDir) + CString(_T("\\")) + CString(s_Dir.name);
                ais.RealFileName = name;
                ServiceLocator::instance()->taskDispatcher()->runInGuiThread([&] {
                    if (m_pWizardDlg->AddImage(ais.RealFileName, ais.VirtualFileName, ais.show)) {
                        count++;
                    }
                }, false);

            }
        }
    } while (_tfindnext(hDir, &s_Dir) == 0);

    _findclose(hDir);

    return 0;
}

DWORD CFolderAdd::Run()
{
    EnableWindow(m_pWizardDlg->m_hWnd, false);
    for (size_t i = 0; i<m_Paths.GetCount(); i++) {
        CString CurPath = m_Paths[i];
        if (WinUtils::IsDirectory(CurPath))
            ProcessDir(CurPath, m_bSubDirs);
        else
            if (!m_bImagesOnly || IuCommonFunctions::IsImage(CurPath)) {
                CWizardDlg::AddImageStruct ais;
                ais.show = !m_pWizardDlg->QuickUploadMarker;
                CString name = CurPath;
                ais.RealFileName = CurPath;
                if (SendMessage(m_pWizardDlg->m_hWnd, WM_MY_ADDIMAGE, (WPARAM)&ais, 0))

                    count++;
            }
        if (dlg.NeedStop()) break;
    }

    dlg.Hide();
    EnableWindow(m_pWizardDlg->m_hWnd, true);

    if (!count)
        MessageBox(m_pWizardDlg->m_hWnd, m_bImagesOnly ? TR("No pictures were found.") : TR("No files were found."), APPNAME, MB_ICONINFORMATION);
    else {
        if (m_pWizardDlg->QuickUploadMarker) {

            m_pWizardDlg->ShowPage(4);
        } else {

            m_pWizardDlg->ShowPage(2);

            ((CMainDlg*)m_pWizardDlg->Pages[2])->ThumbsView.LoadThumbnails();
        }
    }
    dlg.DestroyWindow();
    dlg.m_hWnd = NULL;
    return 0;
}

int CFolderAdd::GetNextImgFile(LPTSTR szBuffer, int nLength)
{
    TCHAR szBuffer2[MAX_PATH], TempPath[256];

    GetTempPath(256, TempPath);
    wsprintf(szBuffer2, _T("%s*.*"), (LPCTSTR)m_szPath);

    if (!findfile) {
        findfile = FindFirstFile(szBuffer2, &wfd);
        if (!findfile) goto error;
    } else {
        if (!FindNextFile(findfile, &wfd))
            goto error;

    }
    if (lstrlen(wfd.cFileName) < 1) goto error;
    lstrcpyn(szBuffer, wfd.cFileName, nLength);

    return TRUE;

error:
    if (findfile) FindClose(findfile);
    return FALSE;
}
