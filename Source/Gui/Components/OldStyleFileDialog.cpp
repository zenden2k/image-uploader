#include "OldStyleFileDialog.h"

#include "Func/WinUtils.h"

COldStyleFileDialog::COldStyleFileDialog(HWND parent, const CString& initialFolder, const CString& title, const FileFilterArray& filters, bool multiselect, bool openDialog) {
    filterStr_.reset(GenerateDialogFilter(filters));

    if (multiselect) {
        multiDlg_.reset(new CMultiFileDialog(0, 0, OFN_HIDEREADONLY, filterStr_.get(), parent));
        multiDlg_->m_ofn.lpstrInitialDir = initialFolder;
    } else {
        dlg_.reset(new CFileDialog(openDialog, 0, 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST, filterStr_.get(), parent));
        dlg_->m_ofn.lpstrInitialDir = initialFolder;
    }
}

INT_PTR COldStyleFileDialog::DoModal(HWND hWndParent) {
    if (multiDlg_) {
        return multiDlg_->DoModal(hWndParent);
    } else {
        return dlg_->DoModal(hWndParent);
    }
}

CString COldStyleFileDialog::getFolderPath() {
    TCHAR buffer[1000]={0};
    if (multiDlg_) {
        multiDlg_->GetDirectory(buffer, sizeof(buffer) / sizeof(TCHAR));
    } else {
        if (dlg_->m_szFileName[0]) {
            WinUtils::ExtractFilePath(dlg_->m_szFileName, buffer);
        }
    }
    return buffer;
}

void COldStyleFileDialog::getFiles(std::vector<CString>& arr) {
    if (multiDlg_) {
        LPCTSTR FileName = nullptr;
        TCHAR Buffer[1000];
        multiDlg_->GetDirectory(Buffer, sizeof(Buffer) / sizeof(TCHAR));

        do {

            FileName = (FileName) ? multiDlg_->GetNextFileName() : multiDlg_->GetFirstFileName();
            if (!FileName) break;
            multiDlg_->GetDirectory(Buffer, sizeof(Buffer) / sizeof(TCHAR));

            if (Buffer[lstrlen(Buffer) - 1] != '\\')
                lstrcat(Buffer, _T("\\"));

            if (FileName) {
                lstrcat(Buffer, FileName);
                arr.push_back(Buffer);
            }
        } while (FileName);
    } else {
        if (dlg_->m_szFileName[0]) {
            arr.push_back(dlg_->m_szFileName);
        }
    }
}

void COldStyleFileDialog::setDefaultExtension(LPCTSTR extension) {
    if (multiDlg_) {
        multiDlg_->m_ofn.lpstrDefExt = extension;
    } else {
        dlg_->m_ofn.lpstrDefExt = extension;
    }
}

void COldStyleFileDialog::setFileTypeIndex(UINT iFileType) {
    if (multiDlg_) {
        multiDlg_->m_ofn.nFilterIndex = iFileType;
    } else {
        dlg_->m_ofn.nFilterIndex = iFileType;
    }
}

void COldStyleFileDialog::setFileName(LPCWSTR fileName) {
    if (!multiDlg_) {
        lstrcpyn(dlg_->m_szFileName, fileName, MAX_PATH-1);
    }
}

WCHAR* COldStyleFileDialog::GenerateDialogFilter(const FileFilterArray& filters) {
    WCHAR* buf = nullptr;
    size_t bufferSize = 0;
    for (const auto& filter : filters) {
        bufferSize += filter.first.GetLength() + 1 + filter.second.GetLength()+1;
    }

    bufferSize += 1;
    buf = new WCHAR[bufferSize];
    WCHAR* szBuffer = buf;

    *szBuffer = 0;

    for (const auto& filter : filters) {
        int nLen = filter.first.GetLength();
        lstrcpy(szBuffer, filter.first);
        szBuffer[nLen] = 0;
        szBuffer += nLen + 1;

        nLen = filter.second.GetLength();
        lstrcpy(szBuffer, filter.second);
        szBuffer[nLen] = 0;
        szBuffer += nLen + 1;
    }
    *szBuffer = 0;
    return buf;
}