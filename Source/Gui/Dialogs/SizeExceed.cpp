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

#include "SizeExceed.h"
#include "atlheaders.h"
#include "Func/common.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Func/myutils.h"
#include "Func/WinUtils.h"
#include "Core/Upload/FileUploadTask.h"

// CSizeExceed
CSizeExceed::CSizeExceed(FileUploadTask * fileTask, CUploadEngineList * EngineList, UploadEngineManager* uploadEngineManager):
        m_EngineList(EngineList)
{
    fileTask_ = fileTask;
    m_szFileName = U2W(fileTask->getFileName());
    uploadEngineManager_ = uploadEngineManager;
}

CSizeExceed::~CSizeExceed()
{
}

void CSizeExceed::Translate()
{
    TRC(IDC_WHATTODO, "Измените настройки изображения или выберите другой сервер, чтобы изображение могло быть загружено.");
    TRC(IDC_IMAGESETTINGS, "Параметры изображения");
    TRC(IDC_FORMATLABEL, "Формат:");
    TRC(IDC_QUALITYLABEL, "Качество:");
    TRC(IDC_RESIZEBYWIDTH, "Изменение ширины:");
    TRC(IDC_SAVEPROPORTIONS, "Сохранять пропорции");
    TRC(IDC_XLABEL, "и/или высоты:");
    TRC(IDOK, "OK");
    TRC(IDC_FORALL, "Для всех");
    TRC(IDCANCEL, "Игнорировать");
    TRC(IDC_SELECTSERVERLABEL, "Сервер для загрузки изображений:");
    TRC(IDC_KEEPASIS, "Оставить без изменения");
    SetWindowText(TR("Превышение размера"));
}

LRESULT CSizeExceed::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rc = {12, 30, 162, 144};
    img.Create(m_hWnd, rc);
    bool isImage = fileTask_->isImage() && IsImage(m_szFileName);
    if (isImage) {
        img.LoadImage(m_szFileName);
    }
    
    CenterWindow(GetParent());

    RECT serverSelectorRect = GuiTools::GetDialogItemRect(m_hWnd, IDC_SERVERPLACEHOLDER);
    imageServerSelector_.reset(new CServerSelectorControl(uploadEngineManager_, true));
    imageServerSelector_->Create(m_hWnd, serverSelectorRect);
    imageServerSelector_->setTitle(fileTask_->isImage() ? TR("Сервер для хранения изображений") : TR("Сервер для хранения других типов файлов"));
    if (!fileTask_->isImage()) {
        imageServerSelector_->setServersMask(CServerSelectorControl::smFileServers);
    }
    
    imageServerSelector_->ShowWindow(SW_SHOW);
    imageServerSelector_->SetWindowPos(0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);
    imageServerSelector_->setServerProfile(Settings.imageServer);
    
    CString serverName = U2W(fileTask_->serverProfile().serverName());


    GuiTools::MakeLabelBold(GetDlgItem(IDC_FILEEXCEEDNAME));
    
    int f = MyGetFileSize(m_szFileName);
    WCHAR buf2[25];
    NewBytesToString(f, buf2, 25);

    CString name;
    CString params; 
    if (isImage) {
        params.Format(_T(" %s (%dx%d, %s)"), (LPCTSTR)myExtractFileName(m_szFileName), (int)img.ImageWidth, (int)img.ImageHeight, (LPCTSTR)buf2);
    } else {
        params.Format(_T(" %s (%s)"), (LPCTSTR)myExtractFileName(m_szFileName), (LPCTSTR)buf2);
    }
   
    name = TR("Файл") + params;

    SetDlgItemText(IDC_FILEEXCEEDNAME, name);
    NewBytesToString(m_EngineList->byName(fileTask_->serverProfile().serverName())->MaxFileSize, buf2, 25);

    TCHAR szBuf[1000];
    wsprintf(szBuf, TR("Файл превышает максимальный размер, допустимый для загрузки на сервер %s (%s)."),
        (LPCTSTR)U2W(fileTask_->serverProfile().serverName()), buf2);
    SetDlgItemText(IDC_FILEEXCEEDSIZE2, szBuf);
    Translate();

    return 1;  // Let the system set the focus
}

LRESULT CSizeExceed::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    fileTask_->setServerProfile(imageServerSelector_->serverProfile());
    EndDialog(wID);
    return 0;
}

LRESULT CSizeExceed::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(wID);
    return 0;
}

LRESULT CSizeExceed::OnBnClickedForall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    fileTask_->setServerProfile(imageServerSelector_->serverProfile());
    EndDialog(3);
    return 0;
}
