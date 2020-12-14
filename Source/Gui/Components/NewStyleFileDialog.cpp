#include "NewStyleFileDialog.h"

typedef HRESULT(STDAPICALLTYPE *SHGetKnownFolderPath_func)(_In_ REFKNOWNFOLDERID rfid,
    _In_ DWORD /* KNOWN_FOLDER_FLAG */ dwFlags,
    _In_opt_ HANDLE hToken,
    _Outptr_ PWSTR *ppszPath); // free *ppszPath with CoTaskMemFree


typedef HRESULT(STDAPICALLTYPE *SHCreateItemFromParsingName_func)(_In_ PCWSTR pszPath, _In_opt_ IBindCtx *pbc, _In_ REFIID riid, _Outptr_ void **ppv);


CNewStyleFileDialog::CNewStyleFileDialog(HWND parent, const CString& initialFolder, const CString& title, const FileFilterArray& filters, bool multiselect, bool openDialog){
    COMDLG_FILTERSPEC* fileTypes = nullptr;
    
    if (!filters.empty()) {
        fileTypes = new COMDLG_FILTERSPEC[filters.size()];
        for (size_t i = 0; i < filters.size(); i++) {
            fileTypes[i].pszName = filters[i].first;
            fileTypes[i].pszSpec = filters[i].second;
        }
    }
    
    DWORD dwFlags = 0;

    // Create the file-save dialog COM object.
    HRESULT hr = newStyleDialog_.CoCreateInstance(CLSID_FileOpenDialog);

    if (FAILED(hr)) {
        delete[] fileTypes;
        return;
    }

    if (fileTypes) {
        newStyleDialog_->SetFileTypes(filters.size(), fileTypes);
    }

    delete[] fileTypes;

    if (!title.IsEmpty()) {
        newStyleDialog_->SetTitle(title);
    }

    CComPtr<IShellItem> psiFolder;


    hr = SHCreateItemFromParsingName(initialFolder, NULL, IID_PPV_ARGS(&psiFolder));

    if (SUCCEEDED(hr)) {
        newStyleDialog_->SetDefaultFolder(psiFolder);
    }

    newStyleDialog_->GetOptions(&dwFlags);

    if (multiselect) {
        dwFlags |= FOS_ALLOWMULTISELECT;
    }
    dwFlags |= FOS_FILEMUSTEXIST | FOS_FORCEFILESYSTEM;
    newStyleDialog_->SetOptions(dwFlags);
}


CNewStyleFileDialog::~CNewStyleFileDialog() {
}

INT_PTR CNewStyleFileDialog::DoModal(HWND hWndParent) {
    HRESULT hr = newStyleDialog_->Show(hWndParent);

    // If the user chose any files, loop thru the array of files.
    if (SUCCEEDED(hr)) {
        return IDOK;
    }
    return IDCANCEL;
}

CString CNewStyleFileDialog::getFolderPath() {
    CString result;

    CComPtr<IShellItem> pFolderItem;
    HRESULT hr = newStyleDialog_->GetFolder(&pFolderItem);
    if (SUCCEEDED(hr)) {
        LPOLESTR pwsz = NULL;

        // Get its file system path.
        hr = pFolderItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

        if (SUCCEEDED(hr)) {
            result = pwsz;
            CoTaskMemFree(pwsz);
        }
    }
    
    return result;
}


void CNewStyleFileDialog::setTitle(LPCWSTR title) {
    newStyleDialog_->SetTitle(title);
}

void CNewStyleFileDialog::setDefaultExtension(LPCTSTR extension) {
    newStyleDialog_->SetDefaultExtension(extension);
}

void CNewStyleFileDialog::setFileName(LPCWSTR fileName) {
    newStyleDialog_->SetFileName(fileName);
}

void CNewStyleFileDialog::getFiles(std::vector<CString>& arr) {
    HRESULT hr;
    // If the user chose any files, loop thru the array of files.

    CComPtr<IShellItemArray> pItemArray;

    hr = newStyleDialog_->GetResults(&pItemArray);

    if (SUCCEEDED(hr)) {
        DWORD cSelItems;

        // Get the number of selected files.
        hr = pItemArray->GetCount(&cSelItems);

        if (SUCCEEDED(hr)) {
            if (!cSelItems) {
                return;
            }
            for (DWORD j = 0; j < cSelItems; j++) {
                CComPtr<IShellItem> pItem;

                // Get an IShellItem interface on the next file.
                hr = pItemArray->GetItemAt(j, &pItem);

                if (SUCCEEDED(hr)) {
                    LPOLESTR pwsz = NULL;

                    // Get its file system path.
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

                    if (SUCCEEDED(hr)) {
                        arr.push_back(pwsz);
                        CoTaskMemFree(pwsz);
                    }
                }
            }
        }
    }
}

void CNewStyleFileDialog::setFileTypeIndex(UINT iFileType) {
    newStyleDialog_->SetFileTypeIndex(iFileType);
}