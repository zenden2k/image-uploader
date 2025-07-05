/*

Uptooda - free application for uploading images/files to the Internet

Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef IU_GUI_NEWSTYLEFOLDERDIALOG_H
#define IU_GUI_NEWSTYLEFOLDERDIALOG_H

#include <memory>
#include "atlheaders.h"

class CNewStyleFolderDialog {
public:

    CNewStyleFolderDialog(HWND parent, const CString& initialFolder, const CString& title, bool onlyFsDirs = true)
    {
        newStyleDialog_= std::make_unique<CShellFileOpenDialog>(/*WinUtils::myExtractFileName(initialFolder)*/nullptr, FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_PICKFOLDERS | (onlyFsDirs ? FOS_FORCEFILESYSTEM : 0));

        if (!initialFolder.IsEmpty()) {
            IShellItem *psi;

            HRESULT hresult = SHCreateItemFromParsingName(initialFolder, nullptr, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hresult)) {
                newStyleDialog_->GetPtr()->SetDefaultFolder(psi);
                psi->Release();
            }
        }
    }

    ~CNewStyleFolderDialog() {
    }

    /*
       This function uses IFileDialog::SetFolder (not SetDefaultFolder) to force initial folder
    */
    void SetFolder(const CString& folder) {
        if (!folder.IsEmpty()) {
            // SHCreateItemFromParsingName function not available on Windows XP
            IShellItem *psi;
            HRESULT hresult = SHCreateItemFromParsingName(folder, nullptr, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hresult)) {
                newStyleDialog_->GetPtr()->SetFolder(psi);
                psi->Release();
            }
        }
    }

    INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
    {
        return newStyleDialog_->DoModal(hWndParent);

    }

    bool AddCheckbox(DWORD controlId, CString title, bool isChecked) {
        CComPtr<IFileDialogCustomize> pfdc;
        HRESULT hr = newStyleDialog_->GetPtr()->QueryInterface(IID_PPV_ARGS(&pfdc));
        if (SUCCEEDED(hr)) {
            hr = pfdc->AddCheckButton(controlId, title, isChecked ? TRUE : FALSE);
            return SUCCEEDED(hr);
        }
        return false;
    }

    bool IsCheckboxChecked(DWORD controlId){
        CComPtr<IFileDialogCustomize> pfdc;
        HRESULT hr = newStyleDialog_->GetPtr()->QueryInterface(IID_PPV_ARGS(&pfdc));
        if (SUCCEEDED(hr)) {
            BOOL isChecked = FALSE;
            hr = pfdc->GetCheckButtonState(controlId, &isChecked);
            if (SUCCEEDED(hr)) {
                return isChecked == TRUE;
            }
        }
        return false;
    }

    CString GetFolderPath()
    {
        CString fileName;
        newStyleDialog_->GetFilePath(fileName);
        return fileName;
    }

    void SetOkButtonLabel(const CString& label) {
        if (newStyleDialog_) {
            newStyleDialog_->GetPtr()->SetOkButtonLabel(label);
        }
    }
protected:
    std::unique_ptr<CShellFileOpenDialog> newStyleDialog_;
};
#endif
