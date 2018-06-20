/*

Image Uploader -  free application for uploading images/files to the Internet

Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "atlheaders.h"
#include "Func/WinUtils.h"
#include "Func/Library.h"

typedef HRESULT(__stdcall *SHCreateItemFromParsingNameFunc)(PCWSTR, IBindCtx *, REFIID, void**);

class CNewStyleFolderDialog {
public:
    
    CNewStyleFolderDialog(HWND parent, const CString& initialFolder, const CString& title, bool onlyFsDirs = true) : shellDll_(L"shell32.dll")
    {
        isVista_ = WinUtils::IsVista();
 
        if (!isVista_ )
        {
            oldStyleDialog_ = new CFolderDialog(parent, title.IsEmpty() ? 0 : static_cast<LPCTSTR>(title), BIF_NEWDIALOGSTYLE | (onlyFsDirs ? BIF_RETURNONLYFSDIRS : 0));
            oldStyleDialog_->SetInitialFolder(initialFolder, true);
            newStyleDialog_ = 0;
        } else {
            newStyleDialog_= new CShellFileOpenDialog(/*WinUtils::myExtractFileName(initialFolder)*/0, FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_PICKFOLDERS | (onlyFsDirs ? FOS_FORCEFILESYSTEM : 0));
            
            // SHCreateItemFromParsingName function not available on Windows XP
            IShellItem *psi;
            SHCreateItemFromParsingNameFunc ParseDisplayName = shellDll_.GetProcAddress<SHCreateItemFromParsingNameFunc>("SHCreateItemFromParsingName");
            HRESULT hresult = ParseDisplayName(initialFolder, 0, IID_IShellItem, reinterpret_cast<void**>(&psi));
            if (SUCCEEDED(hresult)) {
                newStyleDialog_->m_spFileDlg->SetFolder(psi);
                psi->Release();
            }

            oldStyleDialog_ = 0;
        }
    }
    ~CNewStyleFolderDialog()
    {
        delete newStyleDialog_;
        delete oldStyleDialog_;
    }
    INT_PTR DoModal(HWND hWndParent = ::GetActiveWindow())
    {
        if (!isVista_) {
            return oldStyleDialog_->DoModal(hWndParent);
        } else {
            return newStyleDialog_->DoModal(hWndParent);
        }
    }

    CString GetFolderPath()
    {
        if (!isVista_)
        {
            return oldStyleDialog_->GetFolderPath();
        } else {
            CString fileName;
            newStyleDialog_->GetFilePath(fileName);
            return fileName;
        }
    }
protected:
    CFolderDialog* oldStyleDialog_;
    CShellFileOpenDialog* newStyleDialog_;
    bool isVista_;
    Library shellDll_;

};
#endif