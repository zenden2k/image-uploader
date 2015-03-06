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
#include <Wininet.h>
#include <algorithm>
#include <set>
#include <Gui/Controls/CustomEditControl.h>
#include <Func/LocalFileCache.h>
#include <Func/Base.h>
#include <Func/IuCommonFunctions.h>
#include <Gui/Controls/ServerSelectorControl.h>

const TCHAR CImageReuploaderDlg::LogTitle[] = _T("Image Reuploader");

// CImageReuploaderDlg
CImageReuploaderDlg::CImageReuploaderDlg(CWizardDlg *wizardDlg, CMyEngineList * engineList, const CString &initialBuffer)
{
	m_WizardDlg = wizardDlg;
	m_InitialBuffer = initialBuffer;
	m_EngineList = engineList;
	queueUploader_ = new CFileQueueUploader();
	queueUploader_->setCallback( this );
	historySession_ = NULL;
	htmlClipboardFormatId = RegisterClipboardFormat(_T("HTML Format"));
}

CImageReuploaderDlg::~CImageReuploaderDlg()
{
	delete historySession_;
	delete queueUploader_;
	historySession_ = 0;
}

LRESULT CImageReuploaderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	PrevClipboardViewer = SetClipboardViewer();
	DlgResize_Init(false, true, 0); // resizable dialog without "griper"
	sourceTextEditControl.AttachToDlgItem(m_hWnd, IDC_INPUTTEXT);
	sourceTextEditControl.onPaste.bind(this, &CImageReuploaderDlg::OnEditControlPaste);
	outputEditControl.AttachToDlgItem(m_hWnd, IDC_OUTPUTTEXT);
	::SetFocus(GetDlgItem(IDOK));
	SetWindowText(TR("��������� �������� �����������"));
	TRC(IDOK, "����������");
	TRC(IDCANCEL, "�������");
	TRC(IDC_COPYTOCLIPBOARD, "���������� � �����");
	TRC(IDC_SOURCECODERADIO, "�������� ���");
	TRC(IDC_LINKSLISTRADIO, "������ ������");
	TRC(IDC_WATCHCLIPBOARD, "����� ���������� �� ������� ������");
	TRC(IDC_IMAGEDOWNLOADERTIP, "������� ����� (��� HTML, BB-code, ��� ������ ������ URL):");
	TRC(IDC_PASTEHTML, "�������� HTML �� ������");
	TRC(IDC_SOURCEURLLABEL, "URL ��������� (�����������):");
	TRC(IDC_PASTEHTMLONCTRLVCHECKBOX, "��������� HTML ��� ������� Ctrl+V");
	TRC(IDC_DESCRIPTION, "����������� ����������� � ������ � ����������� �������� ��������");
	TRC(IDC_SHOWLOG, "�������� ���");

	RECT serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_IMAGESERVERPLACEHOLDER);
	imageServerSelector_ = new CServerSelectorControl(true);
	imageServerSelector_->Create(m_hWnd, serverSelectorRect);
	imageServerSelector_->setTitle(TR("������ ��� �������� �����������"));
	imageServerSelector_->ShowWindow( SW_SHOW );
	imageServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	imageServerSelector_->setServerProfile(Settings.imageServer);

	::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS), SW_HIDE);
	SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_SETCHECK, Settings.WatchClipboard?BST_CHECKED:BST_UNCHECKED);
	GuiTools::SetCheck(m_hWnd, IDC_SOURCECODERADIO, true );
	GuiTools::SetCheck(m_hWnd, IDC_PASTEHTMLONCTRLVCHECKBOX, Settings.ImageReuploaderSettings.PasteHtmlOnCtrlV);
	GuiTools::MakeLabelBold(GetDlgItem(IDC_DESCRIPTION));
	HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
	if (hWnd) {
		m_wndAnimation.SubclassWindow(hWnd);
		if (m_wndAnimation.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF), _T("GIF")))
			m_wndAnimation.Draw();
		m_wndAnimation.ShowWindow(SW_HIDE);
	}

	if ( IsClipboardFormatAvailable(htmlClipboardFormatId) || IsClipboardFormatAvailable(CF_TEXT ) ) {
		sourceTextEditControl.SendMessage(WM_PASTE);
	} 

	m_serverId = _EngineList->GetUploadEngineIndex(serverProfile_.serverName());

	if(m_serverId == -1) {
		m_serverId = m_EngineList->getRandomImageServer();
		if(m_serverId == -1) {
			return false;
		}
	}
	SetDlgItemText(IDC_SERVENAMELABEL, CString(TR("������")) + _T(": ") + serverProfile_.serverName());

	SetDlgItemText(IDC_RESULTSLABEL, _T(""));
	::SetFocus(GetDlgItem(IDC_FILEINFOEDIT));
	SendDlgItemMessage(IDC_INPUTTEXT, EM_SETLIMITTEXT, 20 * 1024 * 1024, 0); 
	SendDlgItemMessage(IDC_OUTPUTTEXT, EM_SETLIMITTEXT, 20 * 1024 * 1024, 0); 
	return 1; 
}

LRESULT CImageReuploaderDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	serverProfile_ = imageServerSelector_->serverProfile();
	BeginDownloading();
	return 0;
}

LRESULT CImageReuploaderDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	bool closeWindow = true;
	if (m_FileDownloader.IsRunning() ) {
		m_FileDownloader.stop();
		closeWindow = false;
	}
	if ( queueUploader_->IsRunning() ) {
		queueUploader_->stop();
		closeWindow = false;
	}

	if ( closeWindow ) {
		Settings.WatchClipboard = SendDlgItemMessage(IDC_WATCHCLIPBOARD, BM_GETCHECK) != 0;
		OnClose();
		EndDialog(wID);
	}
	return 0;
}

LRESULT CImageReuploaderDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwndRemove = (HWND) wParam;  // handle of window being removed 
	HWND hwndNext = (HWND) lParam;

	if ( hwndRemove == PrevClipboardViewer ) {
		PrevClipboardViewer = hwndNext;
	} else {
		::SendMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
	}
	return 0;
}

void CImageReuploaderDlg::OnDrawClipboard()
{
	bool isHtmlAvailable = IsClipboardFormatAvailable(htmlClipboardFormatId)!=0;

	GuiTools::EnableDialogItem(m_hWnd, IDC_PASTEHTML, isHtmlAvailable);
	//Sending WM_DRAWCLIPBOARD msg to the next window in the chain
	if(PrevClipboardViewer) ::SendMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0); 
}

LRESULT CImageReuploaderDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ChangeClipboardChain(PrevClipboardViewer);
	return 0;
}


bool CImageReuploaderDlg::OnFileFinished(bool ok, int statusCode, CFileDownloader::DownloadFileListItem it)
{
	bool success = false;

	DownloadItemData* dit = reinterpret_cast<DownloadItemData*>(it.id);

	if ( ok ) {		
		m_nFilesDownloaded++;
		if ( addUploadTask(it, it.fileName) ) {
			updateStats();
			success = true;
		}
	} else {

		
	}

	if ( !success ) {
		LogMsgType logMessageType = logError;
		CString cacheLogMessage;
		if ( tryGetFileFromCache(it, cacheLogMessage ) ) {
			logMessageType = logWarning;
		}
		
		WriteLog(logMessageType, LogTitle, _T("Cannot download the image '") + Utf8ToWCstring(it.url) + _T("'.")
			+ _T("\r\nStatus code:") + IntToStr(statusCode) + (cacheLogMessage.IsEmpty() ? _T("") : (_T("\r\n\r\n") + cacheLogMessage)) + _T("\r\n") );

		/*if ( !cacheLogMessage.IsEmpty() ) {
			WriteLog(logWarning, LogTitle, cacheLogMessage);
		}*/
	} 

	return true;
}

bool CImageReuploaderDlg::tryGetFileFromCache(CFileDownloader::DownloadFileListItem it, CString& logMessage) {
	DownloadItemData* dit = reinterpret_cast<DownloadItemData*>(it.id);
	LocalFileCache& localFileCache = LocalFileCache::instance();
	bool success = false;
	localFileCache.ensureHistoryParsed();
	std::string localFile = localFileCache.get(dit->originalUrl);
	CString message;
	if ( !localFile.empty() ) {
		message.Format(_T("File '%s' has been found on local computer ('%s')"), (LPCTSTR)Utf8ToWCstring(it.url), (LPCTSTR)Utf8ToWCstring(localFile) );
		if ( addUploadTask(it, localFile) ) {
			updateStats();
			success = true;
		}
	} else {
		localFile = localFileCache.getThumb(dit->originalUrl);
		if ( !localFile.empty() ) {
			CImageConverter imageConverter;
			Thumbnail thumb;

			if (!thumb.LoadFromFile(WCstringToUtf8(IuCommonFunctions::GetDataFolder() + _T("\\Thumbnails\\") + serverProfile_.getImageUploadParams().getThumb().TemplateName +
				_T(".xml")))) {
				WriteLog(logError, LogTitle, TR("�� ���� ��������� ���� ���������!"));
			} else {
				message.Format(_T("Generating the thumbnail from local file ('%s')"),  (LPCTSTR)Utf8ToWCstring(localFile) );

				CUploadEngineData *ue = m_EngineList->byIndex(m_serverId);
				imageConverter.setEnableProcessing(false);
				imageConverter.setImageConvertingParams(Settings.ConvertProfiles[serverProfile_.getImageUploadParams().ImageProfileName]);
				imageConverter.setThumbCreatingParams(serverProfile_.getImageUploadParams().getThumb());
				bool GenThumbs = serverProfile_.getImageUploadParams().CreateThumbs &&
					((!serverProfile_.getImageUploadParams().UseServerThumbs) || (!ue->SupportThumbnails));
				imageConverter.setThumbnail(&thumb);
				imageConverter.setGenerateThumb(true);
				imageConverter.Convert(Utf8ToWCstring(localFile));

				CString thumbFileName = imageConverter.getThumbFileName();
				if (!thumbFileName.IsEmpty()) {
					if ( addUploadTask(it, WCstringToUtf8(thumbFileName)) ) {
						updateStats();
						success = true;
					}
				}
			}
		}
		//message.Format(_T("File '%s' not found in local cache"), (LPCTSTR)Utf8ToWCstring(it.url) );
	}
	logMessage = message;
	return success;
}

bool CImageReuploaderDlg::addUploadTask(CFileDownloader::DownloadFileListItem it, std::string localFileName ) {
	std::string mimeType = IuCoreUtils::GetFileMimeType(localFileName);
	if (mimeType.find("image/") == std::string::npos) {
		WriteLog(logError, LogTitle, _T("File '") + Utf8ToWCstring(it.url) +
			_T("'\r\n doesn't seems to be an image.\r\nIt has mime type '") + Utf8ToWCstring(mimeType) + "'.");
		return false;
	} 

	DownloadItemData* dit = reinterpret_cast<DownloadItemData*>(it.id);
	CUploadEngineData *ue = serverProfile_.uploadEngineData();
	CUploadEngineData* newData = new CUploadEngineData();
	*newData = *ue;
	CAbstractUploadEngine * e = m_EngineList->getUploadEngine(ue, serverProfile_.serverSettings());
	e->setUploadData(newData);
	ServerSettingsStruct& settings = serverProfile_.serverSettings();
	e->setServerSettings(settings);
	UploadItemData* uploadItemData = new UploadItemData;
	uploadItemData->sourceUrl = it.url;

	uploadItemData->sourceIndex = dit->sourceIndex;
	uploadItemData->originalUrl = dit->originalUrl;

	std::string defaultExtension = IuCoreUtils::GetDefaultExtensionForMimeType(mimeType);
	std::string displayName = it.displayName;
	if ( !defaultExtension.empty() ) {
		std::string fileNameWithoutExt = IuCoreUtils::ExtractFileNameNoExt(it.fileName);
		displayName = fileNameWithoutExt + "." + defaultExtension;
	}
	queueUploader_->AddFile( localFileName, displayName, uploadItemData, e);
	queueUploader_->start();
	return true;
}

void CImageReuploaderDlg::OnQueueFinished()
{
	if(!m_InitialBuffer.IsEmpty())
	{
		OnClose();
		EndDialog(0);
		return;
	}
	if ( !queueUploader_->IsRunning() ) {
		processFinished();
	}
}

bool CImageReuploaderDlg::ExtractLinks(std::string text, std::vector<std::string> &result) {
	pcrepp::Pcre regExpImages("<img [^>]*src=[\"|\']([^\"|\']+)", "imcu");
	
	std::string str = text;
	std::vector<Match> matches;
	std::set<int> uniqueOffsets;

	size_t pos = 0;
	while ( pos <= str.length() ) {
		if ( regExpImages.search(str, pos) ) { 
			Match match;
			match.start = regExpImages.get_match_start(0);
			match.length = regExpImages.get_match_end(0) - match.start + 1;
			
			if ( uniqueOffsets.find(match.start) == uniqueOffsets.end() ) {
				uniqueOffsets.insert(match.start);
				matches.push_back(match);
			}

			pos = regExpImages.get_match_end() + 1;
			
		} else {
			break;
		}
	}

	pcrepp::Pcre regExpBbcode("\\[img\\](.+?)\\[/img\\]", "imcu");

	pos = 0;
	while ( pos <= str.length() ) {
		if ( regExpBbcode.search(str, pos) ) { 
			Match match;
			match.start = regExpBbcode.get_match_start(0);
			match.length = regExpBbcode.get_match_end(0) - match.start + 1;

			if ( uniqueOffsets.find(match.start) == uniqueOffsets.end() ) {
				uniqueOffsets.insert(match.start);
				matches.push_back(match);
			}

			pos = regExpBbcode.get_match_end() + 1;

		} else {
			break;
		}
	}

	pcrepp::Pcre reg("((http|https|ftp)://[\\w\\d:#@%/;$()~_?\\+-=\\\\\\.&]*)", "imcu");
	pos = 0;

	while (pos <= str.length()) {
		if( reg.search(str, pos)) { 
			int questionMarkPos =  reg[0].find_last_of('?');
			std::string url;
			if ( questionMarkPos != std::string::npos ) {
				url = reg[0].substr(0, questionMarkPos);
			} else {
				url = reg[0];
			}
			std::string fileExt = IuStringUtils::toLower( IuCoreUtils::ExtractFileExt(url) );
			if ( fileExt == "png" || fileExt == "jpg" || fileExt == "gif" || fileExt == "jpeg" || fileExt == "bmp" ) { 
				Match match;
				match.start = reg.get_match_start(0);
				match.length = reg.get_match_end(0) - match.start + 1;
				if ( uniqueOffsets.find(match.start) == uniqueOffsets.end() ) {
					uniqueOffsets.insert(match.start);
					matches.push_back(match);
				}
			}
				
			pos = reg.get_match_end() + 1;
		}
		else {
			break;
		}
	}
	std::sort(matches.begin(), matches.end());
	std::unique(matches.begin(), matches.end());

	std::set<std::string> uniqueLinks;

	for ( std::vector<Match>::iterator it = matches.begin(); it != matches.end(); ++it ) {
		std::string link = text.substr(it->start, it->length);
		if ( uniqueLinks.find(link) == uniqueLinks.end() ) {
			uniqueLinks.insert(link);
			result.push_back(link);
		}
	
	}

	return true;
}

bool CImageReuploaderDlg::BeginDownloading()
{
	m_nFilesCount = 0;
	m_nFilesDownloaded = 0;
	m_nFilesUploaded = 0;
	uploadedItems_.clear();
	int index = 0;
	std::string inputText = WCstringToUtf8( GuiTools::GetWindowText(GetDlgItem(IDC_INPUTTEXT)) );
	std::vector<std::string> links;
	ExtractLinks(inputText, links);

	CString sourceUrl = GuiTools::GetDlgItemText( m_hWnd, IDC_SOURCEURLEDIT );
	URL_COMPONENTS urlComponents;
	TCHAR schemeBuffer[MAX_PATH] = _T("\0");

	if ( !sourceUrl.IsEmpty() ) {
		memset(&urlComponents, 0, sizeof(urlComponents));
		urlComponents.lpszScheme = schemeBuffer;
		urlComponents.dwSchemeLength = ARRAY_SIZE(schemeBuffer);
		InternetCrackUrl(sourceUrl, sourceUrl.GetLength(), 0, &urlComponents);

		if ( !lstrlen(urlComponents.lpszScheme) ) {
			CString message;
			message.Format(TR("������������ URL ��������� '%s'. ����� ������ ������ URL, ������� ����� (http,https � �.�.)"), (LPCTSTR)sourceUrl  );
			MessageBox(message, APPNAME, MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
	}

	if ( !links.size() ) {
		MessageBox(TR("�� ������� ������ �� �����������!"), APPNAME, MB_OK | MB_ICONEXCLAMATION);
		return false;
	} else {
		

		std::string result;
		for ( int i = 0; i < links.size(); i++ ) {
			std::string url = links[i];
			std::string fileExt = IuStringUtils::toLower( IuCoreUtils::ExtractFileExt( url ) );

			schemeBuffer[0] = 0;
			CString urlWide = Utf8ToWCstring(url);
			memset(&urlComponents, 0, sizeof(urlComponents));
			urlComponents.lpszScheme = schemeBuffer;
			urlComponents.dwSchemeLength = ARRAY_SIZE(schemeBuffer);
			std::string absoluteUrl = url;

			InternetCrackUrl(urlWide, urlWide.GetLength(), 0, &urlComponents);
			if ( !lstrlen( urlComponents.lpszScheme ) )  {
				if ( sourceUrl.IsEmpty() ) {
					WriteLog(logError, LogTitle, _T("Cannot download file by relative url: \"") + urlWide + _T("\".\r\n You must provide base URL."));
					continue;
				}
				TCHAR absoluteUrlBuffer[MAX_PATH+1];
				DWORD bufferLength = MAX_PATH;
				InternetCombineUrl(sourceUrl, Utf8ToWCstring(url), absoluteUrlBuffer, &bufferLength, 0);
				absoluteUrl = WCstringToUtf8(absoluteUrlBuffer);
			}

			result += absoluteUrl + "\r\n";
			DownloadItemData * dit = new DownloadItemData();
			dit->originalUrl = url;
			dit->sourceIndex = i;
			m_FileDownloader.AddFile( absoluteUrl, reinterpret_cast<void*>(dit), WCstringToUtf8(sourceUrl) );
			m_nFilesCount ++;	

		}
		if ( m_nFilesCount ) {
			m_wndAnimation.ShowWindow(SW_SHOW);
			GuiTools::EnableDialogItem(m_hWnd, IDOK, false);

			SetDlgItemText(IDCANCEL, TR("����������"));
			CHistoryManager * mgr = ZBase::get()->historyManager();

			delete historySession_;			
			historySession_ = mgr->newSession();

			m_FileDownloader.onFileFinished.bind(this, &CImageReuploaderDlg::OnFileFinished);
			m_FileDownloader.onQueueFinished.bind(this, &CImageReuploaderDlg::OnQueueFinished);
			m_FileDownloader.onConfigureNetworkManager.bind(this, &CImageReuploaderDlg::FileDownloader_OnConfigureNetworkManager);
			m_FileDownloader.start();
			
			updateStats();
			//SetDlgItemText( IDC_OUTPUTTEXT,  Utf8ToWCstring(result) );
		}
	}
	return false;
}

bool CImageReuploaderDlg::LinksAvailableInText(const CString &text)
{
	return false;
}


bool CImageReuploaderDlg::OnFileFinished(bool ok,   CFileQueueUploader::FileListItem & result) {
	if ( ok ) {
		UploadItemData* uploadItemData = reinterpret_cast<UploadItemData*>( result.uploadTask->userData );
		UploadedItem item;
		item.sourceUrl   = uploadItemData->sourceUrl;
		item.originalUrl = uploadItemData->originalUrl;
		item.newUrl = result.imageUrl;
		mutex_.acquire();
		uploadedItems_[uploadItemData->sourceIndex] =  item ;
		
		HistoryItem hi;
		hi.localFilePath = result.fileName;
		hi.serverName = result.serverName;
		hi.directUrl =  (result.imageUrl);
		hi.thumbUrl = (result.thumbUrl);
		hi.viewUrl = (result.downloadUrl);
		hi.uploadFileSize = result.fileSize; // IuCoreUtils::getFileSize(WCstringToUtf8(ImageFileName));
		historySession_->AddItem(hi);

		generateOutputText();
		m_nFilesUploaded++;
		mutex_.release();
		updateStats();
	}
	return true;
}

bool CImageReuploaderDlg::OnQueueFinished(CFileQueueUploader*) {
	GuiTools::EnableDialogItem(m_hWnd, IDOK, true);
	updateStats();
	if ( !m_FileDownloader.IsRunning() ) {
		processFinished();
	}
	return true;
}

bool  CImageReuploaderDlg::OnConfigureNetworkManager(CFileQueueUploader* ,NetworkManager* nm) {
	IU_ConfigureProxy(*nm);
	return true;
}

void CImageReuploaderDlg::FileDownloader_OnConfigureNetworkManager(NetworkManager* nm) {
	IU_ConfigureProxy(*nm);
}

void CImageReuploaderDlg::generateOutputText() {
	bool showSourceCode = GuiTools::GetCheck(m_hWnd, IDC_SOURCECODERADIO);

	if ( showSourceCode ) {
		std::string inputText = WCstringToUtf8( GuiTools::GetWindowText(GetDlgItem(IDC_INPUTTEXT)) );
		for ( std::map<unsigned int, UploadedItem>::iterator it =  uploadedItems_.begin(); it  != uploadedItems_.end(); ++it ) {
			UploadedItem& uploadedItem = it->second;
			inputText = IuStringUtils::Replace(inputText, uploadedItem.originalUrl, uploadedItem.newUrl);
		}
		SetDlgItemText( IDC_OUTPUTTEXT,  Utf8ToWCstring(inputText) );
	} else {
		CString outputText;
		for ( std::map<unsigned int, UploadedItem>::iterator it =  uploadedItems_.begin(); it  != uploadedItems_.end(); ++it ) {
			UploadedItem& uploadedItem = it->second;
			outputText += Utf8ToWCstring(uploadedItem.newUrl) + _T("\r\n");
		}
		SetDlgItemText( IDC_OUTPUTTEXT,  outputText );
	}
}

LRESULT CImageReuploaderDlg::OnClickedOutputRadioButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	generateOutputText();
	return 0;
}

void CImageReuploaderDlg::updateStats() {
	CString text =  CString(TR("��������� �����������: ")) + IntToStr(uploadedItems_.size())
		+ (m_nFilesDownloaded ? CString(_T("/")) + IntToStr(m_nFilesDownloaded) : CString(_T("")))
		+ CString(_T(", ")) + TR("�������: ") 
		+ WinUtils::IntToStr(m_nFilesDownloaded) + CString(_T("/")) + WinUtils::IntToStr(m_nFilesCount);
	SetDlgItemText( IDC_RESULTSLABEL,text);
}

void CImageReuploaderDlg::processFinished() {
	::EnableWindow(GetDlgItem(IDOK),true);
	::EnableWindow(GetDlgItem(IDC_FILEINFOEDIT),true);
	TRC(IDCANCEL, "�������");
	::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS),SW_HIDE);
	::EnableWindow(GetDlgItem(IDC_WATCHCLIPBOARD),true);
	m_wndAnimation.ShowWindow(SW_HIDE);
	SetDlgItemText(IDCANCEL, TR("�������"));
}
	
LRESULT CImageReuploaderDlg::OnClickedCopyToClipboardButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	CString outputText = GuiTools::GetWindowText(GetDlgItem(IDC_OUTPUTTEXT));
	WinUtils::CopyTextToClipboard(outputText);
	return 0;
}

bool GetClipboardHtml(CString& text, CString& outSourceUrl)
{
	UINT clipboardFormat = RegisterClipboardFormat(_T("HTML Format"));
	if (OpenClipboard(NULL))
	{
		HGLOBAL hglb = GetClipboardData(clipboardFormat);
		LPCSTR lpstr = (LPCSTR)GlobalLock(hglb);
		//std::string tempStr = lpstr;
		std::string ansiString = (LPCSTR)lpstr;

		std::istringstream f(ansiString);
		std::string line;    
		int startFragment = -1;
		int endFragment = -1;
		std::string sourceUrl;

		while (std::getline(f, line)) {
			std::vector<std::string> tokens;
			IuStringUtils::Split(line, ":", tokens, 2);
			if ( tokens.size() == 2) {
				if ( tokens[0] == "StartFragment") {
					startFragment = atoi(tokens[1].c_str());
				} else if ( tokens[0] == "EndFragment" ) {
					endFragment = atoi(tokens[1].c_str());
				} else if ( tokens[0] == "SourceURL" ) {
					sourceUrl = tokens[1];
				}
			}
			std::cout << line << std::endl;
		}
		if ( startFragment != -1 && endFragment != -1 ) {
			text = Utf8ToWCstring( ansiString.substr(startFragment, endFragment - startFragment) );
		}
		outSourceUrl = Utf8ToWCstring(sourceUrl);

		GlobalUnlock(hglb);
		CloseClipboard();

		return true;
	}

	return false;
}

 #define CF_HTML 2
LRESULT CImageReuploaderDlg::OnClickedPasteHtml(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	//OnEditControlPaste(&sourceTextEditControl);
	pasteHtml();
	//sourceTextEditControl.SendMessage(WM_PASTE, 0, 0);
	return 0;
}

bool CImageReuploaderDlg::OnEditControlPaste(CCustomEditControl*) {
	if ( GuiTools::GetCheck(m_hWnd, IDC_PASTEHTMLONCTRLVCHECKBOX ) ) {
		return pasteHtml();
	}
	return false;
}

bool CImageReuploaderDlg::pasteHtml() {
	if ( IsClipboardFormatAvailable(htmlClipboardFormatId) ) {
		CString clipboardText;
		CString sourceUrl;
		if (  WinUtils::GetClipboardHtml(clipboardText, sourceUrl) ) {
			SendDlgItemMessage(IDC_INPUTTEXT, EM_REPLACESEL, TRUE, (LPARAM)(LPCTSTR)clipboardText);
			//SendDlgItemMessage(IDC_INPUTTEXT, EM_SETSEL, -1, 0);
			SetDlgItemText(IDC_SOURCEURLEDIT, sourceUrl);
			return true;
		} 
		return false;
	}
	return false;
}

bool CImageReuploaderDlg::OnClose() {
	Settings.ImageReuploaderSettings.PasteHtmlOnCtrlV = GuiTools::GetCheck(m_hWnd, IDC_PASTEHTMLONCTRLVCHECKBOX);
	return true;
}

LRESULT CImageReuploaderDlg::OnShowLogClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	LogWindow.Show();
	return 0;
}