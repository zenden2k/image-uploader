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
#ifndef IU_GUI_DIALOGS_LOGWINDOW_H
#define IU_GUI_DIALOGS_LOGWINDOW_H

#pragma once

#include <mutex>
#include <vector>

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/LogListBox.h"
#include "Core/Upload/CommonTypes.h"
#include "Core/Logging/Logger.h"
#include "Func/DefaultLogger.h"

// CLogWindow

class CLogWindow : public CDialogImpl <CLogWindow>,
    public CDialogResize <CLogWindow>,
    public CWinDataExchange <CLogWindow>,
    public CMessageFilter,
    public DefaultLogger::Listener
{
    public:
        struct CLogWndMsg
        {
            ILogger::LogMsgType MsgType;
            CString Sender;
            CString Msg;
            CString Info;
        };

        CLogWindow();
        ~CLogWindow();
        void setLogger(DefaultLogger* logger);
        void setFileNameFilter(CString fileName);
        enum { IDD = IDD_LOGWINDOW };
        enum { IDC_CLEARLIST = 12000, IDC_COPYTEXTTOCLIPBOARD, IDC_SELECTALLITEMS, MYWM_WRITELOG = WM_USER + 100 };
        BOOL PreTranslateMessage(MSG* pMsg) override;

        BEGIN_MSG_MAP(CLogWindow)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_ID_HANDLER(IDC_COPYTEXTTOCLIPBOARD, OnCopyToClipboard)
            MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
            MESSAGE_HANDLER(MYWM_WRITELOG, OnWmWriteLog)
            COMMAND_ID_HANDLER(IDC_CLEARLIST, OnClearList)
            COMMAND_ID_HANDLER(IDC_SELECTALLITEMS, OnSelectAllItems)
            COMMAND_HANDLER(IDC_CLEARLOGBUTTON, BN_CLICKED, OnBnClickedClearLogButtonClicked)
            CHAIN_MSG_MAP(CDialogResize<CLogWindow>)
            REFLECT_NOTIFICATIONS() 
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CLogWindow)
            DLGRESIZE_CONTROL(IDC_MSGLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
            DLGRESIZE_CONTROL(IDC_CLEARLOGBUTTON, DLSZ_MOVE_Y)
        END_DLGRESIZE_MAP()

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        CLogListBox MsgList;
        void Show();
        LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
        LRESULT OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
        LRESULT OnCopyToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
        LRESULT OnSelectAllItems(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        void TranslateUI();
        LRESULT OnBnClickedClearLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        void WriteLog(const DefaultLogger::LogEntry& entry);

        void onItemAdded(size_t index, const DefaultLogger::LogEntry&) override;
        void reloadList();
protected:
    void WriteLogImpl(const DefaultLogger::LogEntry& entry);
    DWORD mainThreadId_;
    std::vector<DefaultLogger::LogEntry> queuedItems_;
    std::mutex queueMutex_;
    DefaultLogger* logger_;
    CString fileNameFilter_;
};

#endif
