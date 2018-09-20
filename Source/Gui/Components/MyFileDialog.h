#ifndef IU_GUI_MYFILEDIALOG_H
#define IU_GUI_MYFILEDIALOG_H

#include "atlheaders.h"
#include <memory>

class IMyFileDialog {
public:
    typedef std::pair<CString, CString> FileFilter;
    typedef std::vector<FileFilter> FileFilterArray;
    virtual ~IMyFileDialog();
    virtual INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())=0;
    virtual CString getFolderPath();
    virtual void setTitle(LPCWSTR title);
    virtual void setFileName(LPCWSTR fileName);
    virtual void setDefaultExtension(LPCTSTR extension);
    virtual void getFiles(std::vector<CString>& arr)=0; 
    CString getFile();
protected:
};

class MyFileDialogFactory {
public:
    static std::shared_ptr<IMyFileDialog> createFileDialog(HWND parent, const CString& initialFolder, const CString& title, const IMyFileDialog::FileFilterArray& filters, bool multiselect = false);
};
#endif