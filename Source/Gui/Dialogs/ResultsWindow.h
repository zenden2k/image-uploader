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

// ResultsWindow.h : Declaration of the CResultsWindow
//
// This dialog window shows technical information
// about  video/audio file that user had selected
#ifndef IU_GUI_RESULTSWINDOW_H
#define IU_GUI_RESULTSWINDOW_H


#pragma once

#include <functional>

#include "atlheaders.h"
#include "resource.h"
#include "ResultsPanel.h"
#include "Gui/Controls/DialogIndirect.h"

class CResultsWindow:     public CDialogIndirectImpl<CResultsWindow>
{
    private:
        CWizardDlg *m_WizardDlg;
    public:
        CResultsWindow(CWizardDlg *wizardDlg, std::vector<ImageUploader::Core::OutputGenerator::UploadObject>  & urlList, bool ChildWindow);
        ~CResultsWindow();

        int GetCodeType() const;
        void UpdateOutput(bool immediately = false);
        void SetCodeType(int Index);
        void Clear();
        void SetPage(ImageUploader::Core::OutputGenerator::CodeLang Index);
        int GetPage();
        void AddServer(const ServerProfile& server);
        void InitUpload();
        void FinishUpload();
        void EnableMediaInfo(bool Enable);
        DLGTEMPLATE* GetTemplate();
        std::mutex& outputMutex();
        void setOnShortenUrlChanged(std::function<void(bool)> fd);
        void setShortenUrls(bool shorten);
        bool copyResultsToClipboard();
        enum { IDD = IDD_RESULTSWINDOW };

    private:
        BEGIN_MSG_MAP(CResultsWindow)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
            MESSAGE_HANDLER(WM_DPICHANGED, OnDpiChanged)
            MESSAGE_HANDLER(WM_MY_DPICHANGED, OnMyDpiChanged)
            NOTIFY_HANDLER(IDC_RESULTSTAB, TCN_SELCHANGE, OnTabChanged)
        END_MSG_MAP()

    private:
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnMyDpiChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        std::unique_ptr<CResultsPanel> ResultsPanel;
        bool m_childWindow;
        CTabCtrl resultsTabCtrl_;
        std::map<int, int> tabPageToCodeLang;
        CIcon iconSmall_;
        HGLOBAL hMyDlgTemplate_;
        void dpiChanged(int dpiX, int dpiY);
};

#endif // IU_GUI_RESULTSWINDOW_H
