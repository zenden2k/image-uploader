#ifndef IU_GUI_OLDSTYLEFILEDIALOG_H
#define IU_GUI_OLDSTYLEFILEDIALOG_H

#include "atlheaders.h"
#include "MyFileDialog.h"
#include <memory>

class COldStyleFileDialog: public IMyFileDialog {
public:
    COldStyleFileDialog(HWND parent, const CString& initialFolder, const CString& title, const FileFilterArray& filters, bool multiselect = false);
    virtual INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow()) override;
    virtual CString getFolderPath() override;
    virtual void getFiles(std::vector<CString>& arr) override; 
protected:
    std::unique_ptr<CMultiFileDialog> multiDlg_;
    std::unique_ptr<CFileDialog> dlg_;
    std::unique_ptr<WCHAR> filterStr_;
    static WCHAR* GenerateDialogFilter(const FileFilterArray& filters);
};

#endif