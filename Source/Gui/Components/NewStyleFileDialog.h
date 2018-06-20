#ifndef IU_GUI_NEWSTYLEFILEDIALOG_H
#define IU_GUI_NEWSTYLEFILEDIALOG_H

#include "atlheaders.h"
#include "MyFileDialog.h"
#include "Func/Library.h"

class CNewStyleFileDialog: public IMyFileDialog {
public:
    CNewStyleFileDialog(HWND parent, const CString& initialFolder, const CString& title, const FileFilterArray& filters, bool multiselect = false);
    ~CNewStyleFileDialog();
    INT_PTR DoModal(HWND hWndParent) override;

    CString getFolderPath() override;
    void setTitle(LPCWSTR title) override;
    void setDefaultExtension(LPCTSTR extension) override;
    void getFiles(std::vector<CString>& arr) override; 
protected:
    bool isVista_;
    Library shellDll_;
    CComPtr<IFileOpenDialog> newStyleDialog_;
};
#endif