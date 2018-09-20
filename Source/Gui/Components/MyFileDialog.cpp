#include "MyFileDialog.h"

#include "Func/WinUtils.h"
#include "NewStyleFileDialog.h"
#include "OldStyleFileDialog.h"

IMyFileDialog::~IMyFileDialog() {
}

void IMyFileDialog::setTitle(LPCWSTR title) {
}

void IMyFileDialog::setFileName(LPCWSTR fileName) {
}

void IMyFileDialog::setDefaultExtension(LPCTSTR extension) {
} 

CString IMyFileDialog::getFolderPath() {
    return CString();
}

CString IMyFileDialog::getFile() {
    std::vector<CString> arr;
    getFiles(arr);
    return arr.empty() ? CString() : arr[0];
}


std::shared_ptr<IMyFileDialog> MyFileDialogFactory::createFileDialog(HWND parent, const CString& initialFolder, const CString& title, const IMyFileDialog::FileFilterArray& filters, bool multiselect) {
    if (WinUtils::IsVista()) {
        return std::shared_ptr<IMyFileDialog>(new CNewStyleFileDialog(parent, initialFolder, title, filters, multiselect));
    } else {
        return std::shared_ptr<IMyFileDialog>(new COldStyleFileDialog(parent, initialFolder, title, filters, multiselect));
    }
}