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

// ImageDownloaderDlg.h : Declaration of the CImageDownloaderDlg
// 
// This dialog window shows technical information 
// about  video/audio file that user had selected
#ifndef IMAGEDOWNLOADERDLG_H
#define IMAGEDOWNLOADERDLG_H


#pragma once

#include "atlheaders.h"
#include "resource.h"       
#include "Core/FileDownloader.h"
#include "WizardDlg.h"
#include "Gui/Controls/DialogIndirect.h"

// CImageDownloaderDlg
class CImageDownloaderDlg : public CCustomDialogIndirectImpl <CImageDownloaderDlg>,
                           public CDialogResize <CImageDownloaderDlg>,
                           public CMessageFilter
{
    public:
        enum { IDD = IDD_IMAGEDOWNLOADER };
        CImageDownloaderDlg(CWizardDlg *wizardDlg /* can be nullptr */, const CString &initialBuffer);
        ~CImageDownloaderDlg() = default;
        const std::vector<CString>& getDownloadedFiles() const;
        int EmulateModal(HWND hWndParent = ::GetActiveWindow(), LPARAM dwInitParam = NULL);

    protected:    
        BEGIN_MSG_MAP(CImageDownloaderDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_ID_HANDLER(IDOK, OnClickedOK)
            COMMAND_ID_HANDLER(IDCANCEL, OnClickedCancel)
            MSG_WM_DRAWCLIPBOARD(OnDrawClipboard)
            MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCbChain)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_CLIPBOARDUPDATE, OnClipboardUpdate) // Windows Vista and later
            CHAIN_MSG_MAP(CDialogResize<CImageDownloaderDlg>)
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CImageDownloaderDlg)
            DLGRESIZE_CONTROL(IDC_FILEINFOEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_DOWNLOADFILESPROGRESS, DLSZ_SIZE_X|DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_IMAGEDOWNLOADERTIP, DLSZ_SIZE_X)
        END_DLGRESIZE_MAP()
        
        
        BOOL EmulateEndDialog(int nRetCode);
        HWND PrevClipboardViewer;
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        void OnDrawClipboard();
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClipboardUpdate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        virtual BOOL PreTranslateMessage(MSG* pMsg) override;
        bool BeginDownloading();
        static bool LinksAvailableInText(const CString &text);
        void ParseBuffer(const CString& text, bool OnlyImages);
        void OnQueueFinished();
        bool OnFileFinished(bool ok, int statusCode, const CFileDownloader::DownloadFileListItem& it);
        void clipboardUpdated();
        CMessageLoop m_loop;
        int m_retCode;
        CString m_FileName;
        CFileDownloader m_FileDownloader;
        CWizardDlg * m_WizardDlg;
        std::vector<CString> m_downloadedFiles;
        int m_nFilesCount;
        int m_nFileDownloaded;
        CString m_InitialBuffer;
        bool isVistaOrLater_;
        CAccelerator accel_;
        typedef BOOL(WINAPI * AddClipboardFormatListenerFunc)(HWND hwnd);
        typedef BOOL(WINAPI * RemoveClipboardFormatListenerFunc)(HWND hwnd);
        RemoveClipboardFormatListenerFunc fRemoveClipboardFormatListener_;
};



#endif // IMAGEDOWNLOADERDLG_H