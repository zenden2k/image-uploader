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
#include "ImageReuploaderDlg.h"
#include "atlheaders.h"
#include "Func/Common.h"
#include "Core/3rdpart/pcreplusplus.h"
#include "LogWindow.h"
#include "Func/Settings.h"
#include "Gui/GuiTools.h"
#include <Core/Utils/StringUtils.h>
#include <Core/Upload/FileQueueUploader.h>
#include <Zthread/Mutex.h>
#include <Func/WinUtils.h>
#include <Func/Myutils.h>

// CImageReuploaderDlg
CImageReuploaderDlg::CImageReuploaderDlg(CWizardDlg *wizardDlg, CMyEngineList * engineList, const CString &initialBuffer)
{
	m_WizardDlg = wizardDlg;
	m_InitialBuffer = initialBuffer;
	m_EngineList = engineList;
	queueUploader_ = new CFileQueueUploader();
	queueUploader_->setCallback( this );
}

CImageReuploaderDlg::~CImageReuploaderDlg()
{
	
}

LRESULT CImageReuploaderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	PrevClipboardViewer = SetClipboardViewer();
	DlgResize_Init(false, true, 0); // resizable dialog without "griper"
 
	::SetFocus(GetDlgItem(IDOK));
	SetWindowText(TR("Повторная загрузка изображений"));
	TRC(IDOK, "Перезалить");
	TRC(IDCANCEL, "Отмена");
	TRC(IDC_WATCHCLIPBOARD, "Вести наблюдение за буфером обмена");
	TRC(IDC_IMAGEDOWNLOADERTIP, "Введите список ссылок (http:// или ftp://, по одной ccылке в строке)");
	::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS), SW_HIDE);
	SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_SETCHECK, Settings.WatchClipboard?BST_CHECKED:BST_UNCHECKED);
	 
	if(!m_InitialBuffer.IsEmpty())
	{
		ParseBuffer(m_InitialBuffer, false);
		BeginDownloading(); 
	}
	::SetFocus(GetDlgItem(IDC_FILEINFOEDIT));
	return 1; 
}



LRESULT CImageReuploaderDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	BeginDownloading();
	return 0;
}

LRESULT CImageReuploaderDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(m_FileDownloader.IsRunning()) 
		m_FileDownloader.stop();
	else
	{
		Settings.WatchClipboard = SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) != 0;
		EndDialog(wID);
	}
	return 0;
}

LRESULT CImageReuploaderDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwndRemove = (HWND) wParam;  // handle of window being removed 
	HWND hwndNext = (HWND) lParam;

	if(hwndRemove == PrevClipboardViewer) PrevClipboardViewer = hwndNext;
	else ::SendMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
	return 0;
}

void CImageReuploaderDlg::OnDrawClipboard()
{
	bool IsClipboard = IsClipboardFormatAvailable(CF_TEXT)!=0;

	if(IsClipboard && SendDlgItemMessage(IDC_WATCHCLIPBOARD,BM_GETCHECK)==BST_CHECKED && !m_FileDownloader.IsRunning()	)
	{
		CString str;  
		WinUtils::GetClipboardText(str);
		ParseBuffer(str, true);
		
	}
	//Sending WM_DRAWCLIPBOARD msg to the next window in the chain
	if(PrevClipboardViewer) ::SendMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0); 
}

LRESULT CImageReuploaderDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ChangeClipboardChain(PrevClipboardViewer);
	return 0;
}


bool CImageReuploaderDlg::OnFileFinished(bool ok, CFileDownloader::DownloadFileListItem it)
{
	if ( ok ) {
			int Server;
		 Server = Settings.ServerID;

		if(Server == -1) {
			Server = m_EngineList->getRandomImageServer();
			if(Server == -1) {
				return false;
			}
		}

		CUploadEngineData *ue = m_EngineList->byIndex(Server);
		CUploadEngineData* newData = new CUploadEngineData();
		*newData =* ue;
		CDefaultUploadEngine *e = new CDefaultUploadEngine();
		e->setUploadData(newData);
		ServerSettingsStruct& settings = Settings.ServerByUtf8Name(newData->Name);
		e->setServerSettings(settings);
		UploadItemData* uploadItemData = new UploadItemData;
		uploadItemData->sourceUrl = it.url;
		queueUploader_->AddFile( it.fileName, it.displayName, uploadItemData, e);
		queueUploader_->start();
	}
	return true;
}

void CImageReuploaderDlg::OnQueueFinished()
{
	if(!m_InitialBuffer.IsEmpty())
	{
		EndDialog(0);
		return;
	}
	::EnableWindow(GetDlgItem(IDOK),true);
	::EnableWindow(GetDlgItem(IDC_FILEINFOEDIT),true);
	TRC(IDCANCEL, "Закрыть");
	SetDlgItemText(IDC_FILEINFOEDIT, _T(""));
	::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS),SW_HIDE);
	::EnableWindow(GetDlgItem(IDC_WATCHCLIPBOARD),true);
}

bool CImageReuploaderDlg::ExtractLinks(std::string text, std::vector<std::string> &result) {
	pcrepp::Pcre reg("((http|https|ftp)://[\\w\\d:#@%/;$()~_?\\+-=\\\\\\.&]*)", "imcu");
	std::string str = text;
	size_t pos = 0;

	while (pos <= str.length()) {
		if( reg.search(str, pos)) { 
			pos = reg.get_match_end()+1;
			result.push_back(reg[0]);
		}
		else {
			break;
		}
	}
	return true;
}

bool CImageReuploaderDlg::BeginDownloading()
{
	int index=0;
	std::string inputText = WCstringToUtf8( GuiTools::GetWindowText(GetDlgItem(IDC_INPUTTEXT)) );
	std::vector<std::string> links;
	ExtractLinks(inputText, links);
	ShowVar( (int)links.size() );
	std::string result;
	for ( int i = 0; i < links.size(); i++ ) {
		std::string url = links[i];
		std::string fileExt = IuStringUtils::toLower( IuCoreUtils::ExtractFileExt( url ) );
		if ( fileExt == "jpg" || fileExt == "png" ) {
			result += links[i] + "\r\n";
			m_FileDownloader.AddFile( url, reinterpret_cast<void*>(i) );
		}

		
	}

	m_FileDownloader.onFileFinished.bind(this, &CImageReuploaderDlg::OnFileFinished);
	m_FileDownloader.onQueueFinished.bind(this, &CImageReuploaderDlg::OnQueueFinished);
	m_FileDownloader.onConfigureNetworkManager.bind(this, &CImageReuploaderDlg::FileDownloader_OnConfigureNetworkManager);
	m_FileDownloader.start();


	SetDlgItemText( IDC_OUTPUTTEXT,  Utf8ToWCstring(result) );
	return false;
}

bool CImageReuploaderDlg::LinksAvailableInText(const CString &text)
{

	return false;
}

void CImageReuploaderDlg::ParseBuffer(const CString& buffer,bool OnlyImages)
{
	
}


bool CImageReuploaderDlg::OnFileFinished(bool ok,   CFileQueueUploader::FileListItem & result) {
	if ( ok ) {
		UploadItemData* uploadItemData = reinterpret_cast<UploadItemData*>( result.uploadTask->userData );
		UploadedItem item;
		item.sourceUrl = uploadItemData->sourceUrl;
		item.newUrl = result.imageUrl;
		mutex_.acquire();
		uploadedItems_.push_back( item );
		generateOutputText();
		mutex_.release();
	}
	return true;
}

bool CImageReuploaderDlg::OnQueueFinished(CFileQueueUploader*) {
	return true;
}

bool  CImageReuploaderDlg::OnConfigureNetworkManager(CFileQueueUploader* ,NetworkManager* nm) {
	return true;
}

void CImageReuploaderDlg::FileDownloader_OnConfigureNetworkManager(NetworkManager* nm) {
}

void CImageReuploaderDlg::generateOutputText() {
	
	std::string inputText = WCstringToUtf8( GuiTools::GetWindowText(GetDlgItem(IDC_INPUTTEXT)) );
	for ( int i =0; i < uploadedItems_.size(); i++ ) {
		UploadedItem& uploadedItem = uploadedItems_[i];
		inputText = IuStringUtils::Replace(inputText, uploadedItem.sourceUrl, uploadedItem.newUrl);
	}
	SetDlgItemText( IDC_OUTPUTTEXT,  Utf8ToWCstring(inputText) );
	
}

bool GetClipboardHtml(CString& text)
{
	UINT clipboardFormat = RegisterClipboardFormat(_T("Rich Text Format"));
	if (OpenClipboard(NULL))
	{
		HGLOBAL hglb = GetClipboardData(clipboardFormat);
		LPCSTR lpstr = (LPCSTR)GlobalLock(hglb);
		//std::string tempStr = lpstr;
		text = /*Utf8ToWCstring(tempStr)*/(LPCWSTR)lpstr;
		GlobalUnlock(hglb);
		CloseClipboard();
			
		return true;
	}

	return false;
}

 #define CF_HTML 2
LRESULT CImageReuploaderDlg::OnClickedPasteHtml(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	UINT clipboardFormat = RegisterClipboardFormat(_T("Rich Text Format"));
	CString text;
	if(IsClipboardFormatAvailable(clipboardFormat)) 
		{
			GetClipboardHtml(text);
			SetDlgItemText(IDC_INPUTTEXT, text);
			//MessageBox(text,0,0);
		}
	return 0;
}