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

#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H
#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/HistoryTreeControl.h"
#include "Gui/Controls/PictureExWnd.h"

class CWizardDlg;
class CHistoryReader;

// CHistoryWindow

#define ID_OPENINBROWSER 13000
#define ID_COPYTOCLIPBOARD ID_OPENINBROWSER + 1
#define ID_VIEWBBCODE ID_OPENINBROWSER + 2
#define ID_OPENFOLDER ID_OPENINBROWSER + 3
#define ID_EDITFILEONSERVER ID_OPENINBROWSER + 4
#define ID_DELETEFILEONSERVER ID_OPENINBROWSER + 5
#define WM_MY_OPENHISTORYFILE WM_USER + 101

class CHistoryWindow : public CDialogImpl <CHistoryWindow>,
    public CDialogResize <CHistoryWindow>,
    public CWinDataExchange <CHistoryWindow>,
    public CMessageFilter
{
    public:
        CHistoryWindow(CWizardDlg* wizardDlg);
        ~CHistoryWindow();
        enum { IDD = IDD_HISTORYWINDOW };
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        BEGIN_MSG_MAP(CHistoryWindow)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
            MESSAGE_HANDLER(WM_MY_OPENHISTORYFILE, OnWmOpenHistoryFile)
            COMMAND_ID_HANDLER(ID_OPENINBROWSER, OnOpenInBrowser)
            COMMAND_ID_HANDLER(ID_COPYTOCLIPBOARD, OnCopyToClipboard)
            COMMAND_ID_HANDLER(ID_VIEWBBCODE, OnViewBBCode)
            COMMAND_ID_HANDLER(ID_OPENFOLDER, OnOpenFolder)
            COMMAND_ID_HANDLER(ID_EDITFILEONSERVER, OnEditFileOnServer)
            COMMAND_ID_HANDLER(ID_DELETEFILEONSERVER, OnDeleteFileOnServer)
            COMMAND_HANDLER(IDC_MONTHCOMBO, CBN_SELCHANGE, OnMonthChanged)
            COMMAND_HANDLER(IDC_DOWNLOADTHUMBS, BN_CLICKED, OnDownloadThumbsCheckboxChecked)
            COMMAND_HANDLER(IDC_CLEARHISTORYBTN, BN_CLICKED, OnBnClickedClearHistoryBtn)
            CHAIN_MSG_MAP(CDialogResize<CHistoryWindow>)
            REFLECT_NOTIFICATIONS()
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CHistoryWindow)
            DLGRESIZE_CONTROL(IDC_HISTORYTREE, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_FILESCOUNTLABEL, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_FILESCOUNTDESCR, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_SESSIONSCOUNTLABEL, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_SESSIONSCOUNTDESCR, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_UPLOADTRAFFICDESCR, DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_UPLOADTRAFFICLABEL, DLSZ_MOVE_Y)
        END_DLGRESIZE_MAP()

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnDownloadThumbsCheckboxChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnWmOpenHistoryFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        void Show();
        void LoadMonthList();
        void FillList(CHistoryReader* mgr);
        CHistoryTreeControl m_treeView;
        CHistoryReader* m_historyReader;
        LRESULT OnHistoryTreeCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
        CString m_delayedFileName;
        void threadsStarted();
        void threadsFinished();
        void onItemDblClick(TreeItem* item);
        std::vector<CString> m_HistoryFiles;
        bool delayed_closing_;
        CString historyFolder;
        CWizardDlg* wizardDlg_;
        void LoadHistoryFile(CString fileName);
        void SelectedMonthChanged();
        void OpenInBrowser(TreeItem* item);
        CPictureExWnd m_wndAnimation;
        // Context menu callbacks
        LRESULT OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnCopyToClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnViewBBCode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnOpenFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnEditFileOnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnDeleteFileOnServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnMonthChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnBnClickedClearHistoryBtn(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

#endif // HISTORYWINDOW_H
