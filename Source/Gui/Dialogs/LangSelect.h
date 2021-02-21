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

#ifndef LANGSELECT_H
#define LANGSELECT_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Controls/MyImage.h"
// CLangSelect


class CLangSelect : public CDialogImpl<CLangSelect>    
{
    public:
        CLangSelect();
        ~CLangSelect();
    
        enum { IDD = IDD_LANGSELECT };
    protected:
        BEGIN_MSG_MAP(CLangSelect)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
            COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
        END_MSG_MAP()

        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
        int GetNextLngFile(LPTSTR szBuffer, int nLength);
        CString Language;

    private:
        WIN32_FIND_DATA wfd;
        HANDLE findfile;
        CMyImage LogoImage;
        CComboBox langListCombo_;
        CFont boldFont_;
        void SelectLang(LPCTSTR Lang);
};


#endif // LANGSELECT_H