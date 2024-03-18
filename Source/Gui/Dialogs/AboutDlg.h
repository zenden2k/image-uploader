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

#pragma once
#include "atlheaders.h"
#include "Gui/Controls/MyImage.h"
#include "Gui/Controls/DialogIndirect.h"
#include "resource.h"

class CAboutDlg : public CCustomDialogIndirectImpl<CAboutDlg>
{
    public:
		// Custom commands should be in the range 0x8000 to 0xDFFF
        enum { IDD = IDD_ABOUTBOX, ID_COPYAUTHOREMAIL = 0x8001 };

    protected:
        BEGIN_MSG_MAP(CAboutDlg)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
            COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
            COMMAND_ID_HANDLER(ID_COPYAUTHOREMAIL, OnCopyAuthorEmail)
            MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
            MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
            CHAIN_MSG_MAP(CCustomDialogIndirectImpl<CAboutDlg>)
        END_MSG_MAP()
        CMyImage LogoImage;
        CHyperLink m_WebSiteLink;
        CHyperLink m_GoogleCodeLink;
        CHyperLink m_ReportBugLink;
        CHyperLink m_EmailLink;
        CHyperLink m_CommitHashLink;
        CHyperLink iconsByIcons8Link;
        CFont thanksToLabelFont_;
        // Handler prototypes (uncomment arguments if needed):
        //    LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
        //    LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
        //    LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

        // Message handlers
        LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
        LRESULT OnCtlColorStatic(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnContextMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
        LRESULT OnCopyAuthorEmail(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

        const wchar_t* authorEmail = L"zenden2k@gmail.com";
};
