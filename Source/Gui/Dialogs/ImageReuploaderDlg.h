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

// ImageDownloaderDlg.h : Declaration of the CImageReuploaderDlg
// 
// This dialog window shows technical information 
// about  video/audio file that user had selected
#ifndef IMAGEREUPLOADERDLG_H
#define IMAGEREUPLOADERDLG_H


#pragma once

#include <mutex>

#include "atlheaders.h"
#include "resource.h"       
#include "Core/FileDownloader.h"
#include "WizardDlg.h"
#include "Core/Upload/FileQueueUploader.h"
#include "Gui/Controls/PictureExWnd.h"
#include "Gui/Controls/CustomEditControl.h"
#include "Core/HistoryManager.h"
#include "Core/Upload/UploadManager.h"
#include "Gui/Controls/DialogIndirect.h"
#include "Gui/Controls/ProgressRingControl.h"

class CFileQueueUploader;
class CMyEngineList;
class CServerSelectorControl;
// CImageReuploaderDlg
class CImageReuploaderDlg : public CCustomDialogIndirectImpl <CImageReuploaderDlg>,
                           public CDialogResize <CImageReuploaderDlg>
{
    public:
        enum { IDD = IDD_IMAGEREUPLOADER };
        CImageReuploaderDlg(CWizardDlg *wizardDlg, CMyEngineList * engineList, UploadManager *  uploadManager, 
            UploadEngineManager *uploadEngineManager,const CString &initialBuffer);
        ~CImageReuploaderDlg() = default;

    protected:    
        BEGIN_MSG_MAP(CImageReuploaderDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_HANDLER(IDC_PASTEHTML, BN_CLICKED, OnClickedPasteHtml)
            COMMAND_HANDLER(IDC_SOURCECODERADIO, BN_CLICKED, OnClickedOutputRadioButton)
            COMMAND_HANDLER(IDC_LINKSLISTRADIO, BN_CLICKED, OnClickedOutputRadioButton)        
            COMMAND_HANDLER(IDC_COPYTOCLIPBOARD, BN_CLICKED, OnClickedCopyToClipboardButton)
            COMMAND_HANDLER(IDC_SHOWLOG, BN_CLICKED, OnShowLogClicked)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_CLIPBOARDUPDATE, OnClipboardUpdate) // Windows Vista and later
            CHAIN_MSG_MAP(CDialogResize<CImageReuploaderDlg>)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CImageReuploaderDlg)
            DLGRESIZE_CONTROL(IDC_INPUTTEXT, DLSZ_SIZE_X)
            DLGRESIZE_CONTROL(IDC_OUTPUTTEXT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
            //DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_COPYTOCLIPBOARD, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_SOURCECODERADIO, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_SHOWLOG, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_LINKSLISTRADIO, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_RESULTSLABEL, DLSZ_SIZE_X)
            DLGRESIZE_CONTROL(IDC_IMAGEDOWNLOADERTIP, DLSZ_SIZE_X)
            DLGRESIZE_CONTROL(IDC_SOURCEURLEDIT, DLSZ_SIZE_X)
        END_DLGRESIZE_MAP()

        struct DownloadItemData {
            std::string originalUrl;
            unsigned int sourceIndex;
        };
        struct UploadedItem {
            std::string sourceUrl;
            std::string newUrl;
            std::string originalUrl;
        };

        struct UploadItemData {
            std::string sourceUrl;
            std::string originalUrl;
            unsigned int sourceIndex;
        };

        typedef BOOL(WINAPI * AddClipboardFormatListenerFunc)(HWND hwnd);
        typedef BOOL(WINAPI * RemoveClipboardFormatListenerFunc)(HWND hwnd);

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedPasteHtml(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedOutputRadioButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCopyToClipboardButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnShowLogClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

        bool ExtractLinks(const std::string& text, std::vector<std::string> &result);
        bool BeginDownloading();
        static bool LinksAvailableInText(const CString &text);
        void OnDownloaderQueueFinished();
        bool OnFileDownloadFinished(bool ok, int statusCode, const CFileDownloader::DownloadFileListItem& it);
        void OnFileFinished(UploadTask* task, bool ok);
        void OnQueueFinished(UploadSession* uploadSession) ;
        bool OnEditControlPaste(CCustomEditControl*);
        void generateOutputText();
        void updateStats();
        void processFinished();
        bool pasteHtml();
        bool OnClose();
        bool tryGetFileFromCache(CFileDownloader::DownloadFileListItem it, CString& logMessage);
        bool addUploadTask(CFileDownloader::DownloadFileListItem it, const std::string& localFileName );
        void clipboardUpdated();
        //bool OnConfigureNetworkClient(NetworkClient* nm);
        // bool OnUploadProgress(UploadProgress progress, UploadTask* task, NetworkClient* nm){return true;}

        CString m_FileName;
        CProgressRingControl m_wndAnimation;
        CFileDownloader m_FileDownloader;
        UploadManager* uploadManager_;
        CMyEngineList *m_EngineList;
        CCustomEditControl sourceTextEditControl, outputEditControl;
        CWizardDlg * m_WizardDlg;
        int m_nFilesCount;
        int m_nFilesDownloaded;
        int m_nFilesUploaded;
        unsigned int htmlClipboardFormatId;
        CString m_InitialBuffer;
        std::mutex mutex_;
        std::shared_ptr<CHistorySession> historySession_;
        std::map<unsigned int, UploadedItem> uploadedItems_;
        ServerProfile serverProfile_;
        static const TCHAR LogTitle[];
        std::unique_ptr<CServerSelectorControl> imageServerSelector_;
        std::shared_ptr<UploadSession> uploadSession_;
        std::mutex uploadSessionMutex_;
        UploadEngineManager *uploadEngineManager_;
        std::vector<std::unique_ptr<DownloadItemData>> downloadItems_; 
        std::vector<std::unique_ptr<UploadItemData>> uploadItems_;
        std::mutex uploadItemsMutex_;

        struct Match {
            int start;
            int length;
            bool operator< (const Match &other ) {
                return start < other.start;
            }
            bool operator== (const Match &other ) {
                return start == other.start;
            }
        };
};



#endif // IMAGEDOWNLOADERDLG_H