#ifndef IU_GUI_NEWSTYLEFILESAVEDIALOG_H
#define IU_GUI_NEWSTYLEFILESAVEDIALOG_H

#include <vector>
#include "atlheaders.h"
#include "MyFileDialog.h"
#include "Func/Library.h"

class CNewStyleFileSaveDialog: public IMyFileDialog {
public:
    CNewStyleFileSaveDialog(HWND parent, const CString& initialFolder, const CString& title, const FileFilterArray& filters);
    ~CNewStyleFileSaveDialog();
    INT_PTR DoModal(HWND hWndParent) override;

    CString getFolderPath() override;
    void setTitle(LPCWSTR title) override;
    void setDefaultExtension(LPCTSTR extension) override;
    void setFileName(LPCWSTR fileName) override;
    void getFiles(std::vector<CString>& arr) override; 
    void setFileTypeIndex(UINT iFileType) override;

protected:
    Library shellDll_{ L"shell32.dll" };
    CComPtr<IFileSaveDialog> newStyleDialog_;
private:
    DISALLOW_COPY_AND_ASSIGN(CNewStyleFileSaveDialog);

};
#endif