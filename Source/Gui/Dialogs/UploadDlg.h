/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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
#ifndef IU_GUI_DIALOGS_UPLOADDLG_H
#define IU_GUI_DIALOGS_UPLOADDLG_H


#pragma once

#include <Shobjidl.h>
#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Core/Upload/Uploader.h"
#include "Gui/Dialogs/maindlg.h"
#include "Gui/Dialogs/ResultsWindow.h"
#include "Core/Upload/FileQueueUploader.h"
#include "Core/Settings.h"
#include "Gui/Controls/ResultsListView.h"
#include "Func/MyEngineList.h"

class UploadManager;

class CUploadDlg : public CDialogImpl<CUploadDlg>,
                         public CWizardPage
{
    public:
        CUploadDlg(CWizardDlg *dlg, UploadManager* uploadManager);
        ~CUploadDlg();
        enum { IDD = IDD_UPLOADDLG };
        enum {
            IDC_UPLOADPROCESSTAB = WM_USER + 100, IDC_UPLOADRESULTSTAB = IDC_UPLOADPROCESSTAB + 1,
            kEnableNextButtonTimer = 5,
            kProgressTimer = 6
        };
        
         BEGIN_MSG_MAP(CUploadDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_TIMER, OnTimer)
            COMMAND_HANDLER(IDC_UPLOADPROCESSTAB, BN_CLICKED, OnUploadProcessButtonClick)
            COMMAND_HANDLER(IDC_UPLOADRESULTSTAB, BN_CLICKED, OnUploadResultsButtonClick)
            COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
        END_MSG_MAP()

         // Handler prototypes:
         //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
         //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
         //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        bool startUpload();
        CMainDlg *MainDlg;
        std::unique_ptr<CResultsWindow> resultsWindow_;
        int ThreadTerminated(void);
        std::vector<CUrlListItem> urlList_;
        bool OnShow() override;
        bool OnNext() override;
        bool OnHide() override;
        bool CancelByUser;
        void GenerateOutput();
        void TotalUploadProgress(int CurPos, int Total,int FileProgress=0);
        int progressCurrent, progressTotal;
        CMyEngineList *engineList_;
        
        void OnUploaderStatusChanged(UploadTask* task);
        // Is called when upload engine is uploading to remote folder(album)
        // Then this album will appear in dropdown list "Options"
        void OnFolderUsed(UploadTask* task);
        void onShortenUrlChanged(bool shortenUrl);
    protected:
        LRESULT OnUploadProcessButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnUploadResultsButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        void showUploadResultsTab();
        void showUploadProgressTab();
        void onSessionFinished(UploadSession* session);
        void onSessionFinished_UiThread(UploadSession* session);
        void onTaskUploadProgress(UploadTask* task);
        void onTaskFinished(UploadTask* task, bool ok);
        void onChildTaskAdded(UploadTask* child);
        void backgroundThreadStarted();
        void createToolbar();
        void updateTotalProgress();
        int currentTab_;
        CResultsListView uploadListView_;
        bool isEnableNextButtonTimerRunning_;
        std::shared_ptr<UploadSession> uploadSession_;
        struct FileProcessingStruct {
            CString fileName;
            int tableRow;
        };
        std::vector<FileProcessingStruct*> files_;
        //std::mutex uploadListViewMutex_;
        bool alreadyShortened_;
        ServerProfile sessionImageServer_, sessionFileServer_;
        bool backgroundThreadStarted_;
        std::mutex backgroundThreadStartedMutex_;
        
        #if  WINVER    >= 0x0601
                ITaskbarList3* ptl;
        #endif
        UploadManager* uploadManager_;
        CToolBarCtrl toolbar_;
        CFont commonProgressLabelFont_, commonPercentLabelFont_;
        CImageList toolbarImageList_;
};



#endif // IU_GUI_DIALOGS_UPLOADDLG_H