#include "MyFileDialog.h"

#include <cassert>

#include "Func/WinUtils.h"
#include "NewStyleFileDialog.h"
#include "OldStyleFileDialog.h"
#include "NewStyleFileSaveDialog.h"


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

void IMyFileDialog::setFileTypeIndex(UINT iFileType) {
}

CString IMyFileDialog::getFile() {
    std::vector<CString> arr;
    getFiles(arr);
    return arr.empty() ? CString() : arr[0];
}


std::unique_ptr<IMyFileDialog> MyFileDialogFactory::createFileDialog(HWND parent, const CString& initialFolder, const CString& title, const IMyFileDialog::FileFilterArray& filters, bool multiselect, bool openDialog) {
    if (multiselect) {
        assert(openDialog==true);
    }
    if (WinUtils::IsVistaOrLater()) {
        if (openDialog) {
            return std::make_unique<CNewStyleFileDialog>(parent, initialFolder, title, filters, multiselect, openDialog);
        } else {
            return std::make_unique<CNewStyleFileSaveDialog>(parent, initialFolder, title, filters);
        }
    } else {
        return std::make_unique<COldStyleFileDialog>(parent, initialFolder, title, filters, multiselect, openDialog);
    }
}