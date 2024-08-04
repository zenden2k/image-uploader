/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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
#ifndef SEARCHBYIMAGEDLG_H
#define SEARCHBYIMAGEDLG_H

#pragma once
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/SearchByImage.h"
#include "Gui/Controls/DialogIndirect.h"
#include "Gui/Controls/ProgressRingControl.h"

// CSearchByImageDlg
class SearchByImage;

class CSearchByImageDlg :
    public CCustomDialogIndirectImpl<CSearchByImageDlg>
{
    public:
        explicit CSearchByImageDlg(UploadManager* uploadManager, const ServerProfile& searchEngine, CString fileName);
        ~CSearchByImageDlg();
        enum { IDD = IDD_SEARCHBYIMAGEDLG};

        BEGIN_MSG_MAP(CSearchByImageDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        END_MSG_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
private:
    CString fileName_;
    std::shared_ptr<UploadSession> session_;
    bool cancelPressed_;
    std::atomic_bool finished_;
    CProgressRingControl wndAnimation_;
    ServerProfile searchEngine_;
    UploadManager* uploadManager_;
    CFont titleLabelFont_;
    void onSeekerFinished(bool success, const CString& msg);
};

#endif // STATUSDLG_H
