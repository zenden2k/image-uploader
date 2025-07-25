#ifndef IU_GUI_DAILOGS_FOLDERADDDLG_H
#define IU_GUI_DAILOGS_FOLDERADDDLG_H

#pragma once
#include "atlheaders.h"
#include "3rdpart/thread.h"
#include "StatusDlg.h"

class CWizardDlg;

class CFolderAdd : public CThreadImpl<CFolderAdd> {
public:
    CFolderAdd(CWizardDlg *WizardDlg);
    void Do(CStringList &Paths, bool ImagesOnly, bool SubDirs = false);
    DWORD Run();
private:
    size_t count;
    CStringList m_Paths;
    bool m_bSubDirs;
    bool m_bImagesOnly;
    CWizardDlg *m_pWizardDlg;
    WIN32_FIND_DATA wfd;
    HANDLE findfile;
    std::shared_ptr<CStatusDlg> dlg;
    int ProcessDir(CString currentDir, bool bRecursive /* = true  */);
};

#endif
