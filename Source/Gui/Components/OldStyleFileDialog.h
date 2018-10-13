#ifndef IU_GUI_OLDSTYLEFILEDIALOG_H
#define IU_GUI_OLDSTYLEFILEDIALOG_H

#include "atlheaders.h"
#include "MyFileDialog.h"
#include <memory>

class COldStyleFileDialog: public IMyFileDialog {
public:
    COldStyleFileDialog(HWND parent, const CString& initialFolder, const CString& title, const FileFilterArray& filters, bool multiselect = false, bool openDialog = true);
    virtual INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow()) override;
    virtual CString getFolderPath() override;
    virtual void getFiles(std::vector<CString>& arr) override; 
    virtual void setDefaultExtension(LPCTSTR extension) override;
    virtual void setFileTypeIndex(UINT iFileType) override;
    void setFileName(LPCWSTR fileName) override;
protected:
    std::unique_ptr<CMultiFileDialog> multiDlg_;
    std::unique_ptr<CFileDialog> dlg_;
    std::unique_ptr<WCHAR> filterStr_;
    static WCHAR* GenerateDialogFilter(const FileFilterArray& filters);
private:
    DISALLOW_COPY_AND_ASSIGN(COldStyleFileDialog);
};

#endif