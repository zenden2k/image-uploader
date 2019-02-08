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

#include "DefaultServersSettings.h"

#include "Func/common.h"
#include "Core/Settings.h"
#include "Gui/GuiTools.h"
#include "Gui/Controls/ServerSelectorControl.h"
#include "WizardDlg.h"
#include "Core/ServiceLocator.h"

// CDefaultServersSettings
CDefaultServersSettings::CDefaultServersSettings(UploadEngineManager* uploadEngineManager)
{
    uploadEngineManager_ = uploadEngineManager;
}

CDefaultServersSettings::~CDefaultServersSettings()
{
}

LRESULT CDefaultServersSettings::OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    imageServerSelector_->updateServerList();
    fileServerSelector_->updateServerList();
    trayServerSelector_->updateServerList();
    contextMenuServerSelector_->updateServerList();
    temporaryServerSelector_->updateServerList();
    return 0;
}

LRESULT CDefaultServersSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    TRC(IDC_REMEMBERIMAGESERVERSETTINGS, "Remember image server's settings in wizard");
    TRC(IDC_REMEMBERFILESERVERSETTINGS, "Remember file server's settings in wizard");
    RECT serverSelectorRect;
    serverSelectorRect = GuiTools::GetDialogItemRect(m_hWnd, IDC_IMAGESERVERPLACEHOLDER);
    imageServerSelector_ = std::make_unique<CServerSelectorControl>(uploadEngineManager_, true);
    if ( !imageServerSelector_->Create(m_hWnd, serverSelectorRect) ) {
        return 0;
    }
    imageServerSelector_->setTitle(TR("Default server for uploading images"));
    imageServerSelector_->ShowWindow( SW_SHOW );
    imageServerSelector_->SetWindowPos(GetDlgItem(IDC_IMAGESERVERPLACEHOLDER), serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);
    imageServerSelector_->setServerProfile(Settings.imageServer);

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_FILESERVERPLACEHOLDER);

    fileServerSelector_ = std::make_unique<CServerSelectorControl>(uploadEngineManager_);
    fileServerSelector_->setServersMask(CServerSelectorControl::smFileServers);
    fileServerSelector_->setShowImageProcessingParams(false);
    fileServerSelector_->Create(m_hWnd, serverSelectorRect);
    fileServerSelector_->ShowWindow( SW_SHOW );
    fileServerSelector_->SetWindowPos(GetDlgItem(IDC_FILESERVERPLACEHOLDER), serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);
    fileServerSelector_->setServerProfile(Settings.fileServer);
    fileServerSelector_->setTitle(TR("Default server for other file types"));

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_TRAYSERVERPLACEHOLDER);
    trayServerSelector_ = std::make_unique<CServerSelectorControl>(uploadEngineManager_);
    //trayServerSelector_->setShowDefaultServerItem(true);
    trayServerSelector_->Create(m_hWnd, serverSelectorRect);
    trayServerSelector_->ShowWindow( SW_SHOW );
    trayServerSelector_->SetWindowPos(GetDlgItem(IDC_TRAYSERVERPLACEHOLDER), serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);

    trayServerSelector_->setServerProfile(Settings.quickScreenshotServer);
    trayServerSelector_->setTitle(TR("Server for quick screenshot uploading"));

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_CONTEXTMENUSERVERPLACEHOLDER);

    contextMenuServerSelector_  = std::make_unique<CServerSelectorControl>(uploadEngineManager_);
    //contextMenuServerSelector_->setShowDefaultServerItem(true);
    contextMenuServerSelector_->Create(m_hWnd, serverSelectorRect);
    contextMenuServerSelector_->ShowWindow( SW_SHOW );
    contextMenuServerSelector_->SetWindowPos(GetDlgItem(IDC_CONTEXTMENUSERVERPLACEHOLDER), serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);

    contextMenuServerSelector_->setServerProfile(Settings.contextMenuServer);
    contextMenuServerSelector_->setTitle(TR("Server for uploading from Explorer's context menu"));


    // Intermediate server for storing temporary images
    serverSelectorRect = GuiTools::GetDialogItemRect(m_hWnd, IDC_TEMPORARYSERVERPLACEHOLDER);
    temporaryServerSelector_ = std::make_unique<CServerSelectorControl>(uploadEngineManager_);
    //trayServerSelector_->setShowDefaultServerItem(true);
    temporaryServerSelector_->Create(m_hWnd, serverSelectorRect);
    temporaryServerSelector_->ShowWindow(SW_SHOW);
    temporaryServerSelector_->SetWindowPos(GetDlgItem(IDC_TEMPORARYSERVERPLACEHOLDER), serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);

    temporaryServerSelector_->setServerProfile(Settings.temporaryServer);
    temporaryServerSelector_->setTitle(TR("Server for temporary images"));

    serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_URLSHORTENERPLACEHOLDER);

    urlShortenerServerSelector_ = std::make_unique<CServerSelectorControl>(uploadEngineManager_);
    urlShortenerServerSelector_->setServersMask(CServerSelectorControl::smUrlShorteners);
    urlShortenerServerSelector_->setShowImageProcessingParams(false);
    urlShortenerServerSelector_->setShowParamsLink(false);
    urlShortenerServerSelector_->Create(m_hWnd, serverSelectorRect);
    urlShortenerServerSelector_->ShowWindow( SW_SHOW );
    urlShortenerServerSelector_->SetWindowPos(GetDlgItem(IDC_URLSHORTENERPLACEHOLDER), serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right - serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top, 0);
    urlShortenerServerSelector_->setServerProfile(Settings.urlShorteningServer);
    urlShortenerServerSelector_->setTitle(TR("URL shortening server"));


    GuiTools::SetCheck(m_hWnd, IDC_REMEMBERIMAGESERVERSETTINGS, Settings.RememberImageServer);
    GuiTools::SetCheck(m_hWnd, IDC_REMEMBERFILESERVERSETTINGS, Settings.RememberFileServer);

    return 1;  // Let the system set the focus
}
 
bool CDefaultServersSettings::Apply()
{
    CServerSelectorControl* controls[] = { fileServerSelector_.get(), imageServerSelector_.get(), trayServerSelector_.get(),
        contextMenuServerSelector_.get(), urlShortenerServerSelector_.get(), temporaryServerSelector_.get() };
    for(int i = 0; i< ARRAY_SIZE(controls); i++ ) {
        if (controls[i]->serverProfile().serverName().empty()) {
            CString message;
            message.Format(TR("You have not selected \"%s\""), controls[i]->getTitle());
            Lang.LocalizedMessageBox(m_hWnd, message, TR("Error"), MB_ICONERROR);
            return 0;
        } else if ( !controls[i]->isAccountChosen() ) {
            CString message;
            message.Format(TR("You have not selected account for server \"%s\""), IuCoreUtils::Utf8ToWstring(controls[i]->serverProfile().serverName()).c_str());
            Lang.LocalizedMessageBox(m_hWnd, message, TR("Error"), MB_ICONERROR);
            return 0;
        }
    }
    Settings.fileServer = fileServerSelector_->serverProfile();
    Settings.imageServer = imageServerSelector_->serverProfile();
    Settings.quickScreenshotServer = trayServerSelector_->serverProfile();
    Settings.contextMenuServer = contextMenuServerSelector_->serverProfile();
    Settings.urlShorteningServer = urlShortenerServerSelector_->serverProfile();
    Settings.temporaryServer = temporaryServerSelector_->serverProfile();
    Settings.RememberImageServer = GuiTools::GetCheck(m_hWnd, IDC_REMEMBERIMAGESERVERSETTINGS);
    Settings.RememberFileServer = GuiTools::GetCheck(m_hWnd, IDC_REMEMBERFILESERVERSETTINGS);
    ServiceLocator::instance()->programWindow()->setServersChanged(true);
    return true;
}

