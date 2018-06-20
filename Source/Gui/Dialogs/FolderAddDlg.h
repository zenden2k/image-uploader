#ifndef IU_GUI_DAILOGS_FOLDERADDDLG_H
#define IU_GUI_DAILOGS_FOLDERADDDLG_H

#pragma once
#include "3rdpart/thread.h"
#include "StatusDlg.h"

class CWizardDlg;

class CFolderAdd : public CThreadImpl<CFolderAdd> {
public:
    CFolderAdd(CWizardDlg *WizardDlg);
    void Do(CStringList &Paths, bool ImagesOnly, bool SubDirs = false);
    DWORD Run();
private:
    int count;
    CStringList m_Paths;
    bool m_bSubDirs;
    bool m_bImagesOnly;
    CWizardDlg *m_pWizardDlg;
    TCHAR m_szPath[MAX_PATH];
    WIN32_FIND_DATA wfd;
    HANDLE findfile;
    CStatusDlg dlg;
    int GetNextImgFile(LPTSTR szBuffer, int nLength);
    int ProcessDir(CString currentDir, bool bRecursive /* = true  */);
};

#endif