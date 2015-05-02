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
#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/loglistbox.h"
#include <atlddx.h>
#include <atlframe.h>
#include "Core/Upload/CommonTypes.h"

#define MYWM_WRITELOG WM_USER + 100

// CLogWindow

class CLogWindow : public CDialogImpl <CLogWindow>,
    public CDialogResize <CLogWindow>,
    public CWinDataExchange <CLogWindow>,
    public CMessageFilter
{
    public:
        struct CLogWndMsg
        {
            LogMsgType MsgType;
            CString Sender;
            CString Msg;
            CString Info;
        };

    public:
        CLogWindow();
        ~CLogWindow();
        enum { IDD = IDD_LOGWINDOW };
        enum { IDC_CLEARLIST = 12000, IDC_COPYTEXTTOCLIPBOARD};
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        BEGIN_MSG_MAP(CLogWindow)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
            COMMAND_ID_HANDLER(IDC_COPYTEXTTOCLIPBOARD, OnCopyToClipboard)
            MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
            MESSAGE_HANDLER(MYWM_WRITELOG, OnWmWriteLog)
            COMMAND_ID_HANDLER(IDC_CLEARLIST, OnClearList)
            COMMAND_HANDLER(IDC_CLEARLOGBUTTON, BN_CLICKED, OnBnClickedClearLogButtonClicked)
            CHAIN_MSG_MAP(CDialogResize<CLogWindow>)
            REFLECT_NOTIFICATIONS() 
        END_MSG_MAP()

        BEGIN_DLGRESIZE_MAP(CLogWindow)
            DLGRESIZE_CONTROL(IDC_MSGLIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
            DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
        END_DLGRESIZE_MAP()

        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnWmWriteLog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        void WriteLog(LogMsgType MsgType, const CString& Sender, const CString& Msg, const CString& Info = CString() );
        CLogListBox MsgList;
        void Show();
        LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
        LRESULT OnClearList(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
        LRESULT OnCopyToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
        void TranslateUI();
        LRESULT OnBnClickedClearLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

extern CLogWindow LogWindow;

void WriteLog(LogMsgType MsgType, const CString& Sender,  const CString&  Msg,  const CString&  Info = CString());

namespace DefaultErrorHandling {
void ErrorMessage(ErrorInfo);
void DebugMessage(const std::string&, bool);

};

#endif // LOGWINDOW_H
