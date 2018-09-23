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

#ifndef DefaultServersSettings_H
#define DefaultServersSettings_H

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Gui/Dialogs/settingspage.h"
#include "Gui/COntrols/ServerSelectorControl.h"
class CServerSelectorControl;
class UploadEngineManager;
class CDefaultServersSettings : public CDialogImpl<CDefaultServersSettings>, 
                          public CSettingsPage    
{
    public:
        enum { IDD = IDD_DEFAULTSERVERSSETTINGS };

        CDefaultServersSettings(UploadEngineManager* uploadEngineManager);
        virtual ~CDefaultServersSettings();
        virtual bool Apply();

    protected:
        BEGIN_MSG_MAP(CDefaultServersSettings)
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER(WM_SERVERSELECTCONTROL_SERVERLIST_CHANGED, OnServerListChanged)
            
        END_MSG_MAP()
        // Handler prototypes:
        //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
        //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
        LRESULT OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
    CServerSelectorControl *imageServerSelector_;
    CServerSelectorControl *fileServerSelector_; 
    CServerSelectorControl *trayServerSelector_;
    CServerSelectorControl *contextMenuServerSelector_;
    CServerSelectorControl *urlShortenerServerSelector_;
    CServerSelectorControl *temporaryServerSelector_;
    UploadEngineManager* uploadEngineManager_;

};

#endif // DefaultServersSettings_H

