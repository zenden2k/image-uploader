/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DefaultServersSettings.h"

#include <uxtheme.h>
#include "Func/common.h"
#include "Func/Settings.h"
#include "LogWindow.h"
#include "Gui/GuiTools.h"
#include <Func/WinUtils.h>
#include <Gui/Controls/ServerSelectorControl.h>
#include "WizardDlg.h"

// CDefaultServersSettings
CDefaultServersSettings::CDefaultServersSettings()
{
}

CDefaultServersSettings::~CDefaultServersSettings()
{
	delete fileServerSelector_;
	delete imageServerSelector_;
	delete trayServerSelector_;
	delete contextMenuServerSelector_;
	delete urlShortenerServerSelector_;
}

LRESULT CDefaultServersSettings::OnServerListChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	imageServerSelector_->updateServerList();
	fileServerSelector_->updateServerList();
	trayServerSelector_->updateServerList();
	contextMenuServerSelector_->updateServerList();
	return 0;
}

LRESULT CDefaultServersSettings::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_IMAGESERVERPLACEHOLDER);
	imageServerSelector_ = new CServerSelectorControl(true);
	imageServerSelector_->Create(m_hWnd, serverSelectorRect);
	imageServerSelector_->setTitle(TR("Сервер по-умолчанию для хранения изображений"));
	imageServerSelector_->ShowWindow( SW_SHOW );
	imageServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	imageServerSelector_->setServerProfile(Settings.imageServer);

	serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_FILESERVERPLACEHOLDER);

	fileServerSelector_ = new CServerSelectorControl();
	fileServerSelector_->setServersMask(CServerSelectorControl::smFileServers);
	fileServerSelector_->setShowImageProcessingParamsLink(false);
	fileServerSelector_->Create(m_hWnd, serverSelectorRect);
	fileServerSelector_->ShowWindow( SW_SHOW );
	fileServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	fileServerSelector_->setServerProfile(Settings.fileServer);
	fileServerSelector_->setTitle(TR("Сервер по-умолчанию для хранения других типов файлов"));

	serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_TRAYSERVERPLACEHOLDER);
	trayServerSelector_ = new CServerSelectorControl();
	//trayServerSelector_->setShowDefaultServerItem(true);
	trayServerSelector_->Create(m_hWnd, serverSelectorRect);
	trayServerSelector_->ShowWindow( SW_SHOW );
	trayServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);

	trayServerSelector_->setServerProfile(Settings.quickScreenshotServer);
	trayServerSelector_->setTitle(TR("Сервер для быстрой загрузки скриншотов"));

	serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_CONTEXTMENUSERVERPLACEHOLDER);


	contextMenuServerSelector_ = new CServerSelectorControl();
	//contextMenuServerSelector_->setShowDefaultServerItem(true);
	contextMenuServerSelector_->Create(m_hWnd, serverSelectorRect);
	contextMenuServerSelector_->ShowWindow( SW_SHOW );
	contextMenuServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);

	contextMenuServerSelector_->setServerProfile(Settings.contextMenuServer);
	contextMenuServerSelector_->setTitle(TR("Сервер для загрузки из контекстного меню проводника"));


	serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_URLSHORTENERPLACEHOLDER);

	urlShortenerServerSelector_ = new CServerSelectorControl();
	urlShortenerServerSelector_->setServersMask(CServerSelectorControl::smUrlShorteners);
	urlShortenerServerSelector_->setShowImageProcessingParamsLink(false);
	urlShortenerServerSelector_->Create(m_hWnd, serverSelectorRect);
	urlShortenerServerSelector_->ShowWindow( SW_SHOW );
	urlShortenerServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	urlShortenerServerSelector_->setServerProfile(Settings.urlShorteningServer);
	urlShortenerServerSelector_->setTitle(TR("Сервер для сокращения ссылок"));
	
	GuiTools::SetCheck(m_hWnd, IDC_REMEMBERIMAGESERVERSETTINGS, Settings.RememberImageServer);
	GuiTools::SetCheck(m_hWnd, IDC_REMEMBERFILESERVERSETTINGS, Settings.RememberFileServer);


	
	
	return 1;  // Let the system set the focus
}


	
bool CDefaultServersSettings::Apply()
{
	Settings.fileServer = fileServerSelector_->serverProfile();
	Settings.imageServer = imageServerSelector_->serverProfile();
	Settings.quickScreenshotServer = trayServerSelector_->serverProfile();
	Settings.contextMenuServer = contextMenuServerSelector_->serverProfile();
	Settings.urlShorteningServer = urlShortenerServerSelector_->serverProfile();
	Settings.RememberImageServer = GuiTools::GetCheck(m_hWnd, IDC_REMEMBERIMAGESERVERSETTINGS);
	Settings.RememberFileServer = GuiTools::GetCheck(m_hWnd, IDC_REMEMBERFILESERVERSETTINGS);
	pWizardDlg->setServersChanged(true);
	return true;
}

