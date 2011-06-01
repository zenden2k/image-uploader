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
#include "ImageDownloaderDlg.h"
#include "atlheaders.h"
#include "Func/Common.h"
#include "Core/3rdpart/pcreplusplus.h"
#include "LogWindow.h"
#include "Func/Settings.h"

// CImageDownloaderDlg
CImageDownloaderDlg::CImageDownloaderDlg(CWizardDlg *wizardDlg,const CString &initialBuffer)
{
	m_WizardDlg = wizardDlg;
	m_InitialBuffer = initialBuffer;
}

CImageDownloaderDlg::~CImageDownloaderDlg()
{
	
}

LRESULT CImageDownloaderDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent());
	PrevClipboardViewer = SetClipboardViewer();
	DlgResize_Init(false, true, 0); // resizable dialog without "griper"
 
	::SetFocus(GetDlgItem(IDOK));
	SetWindowText(TR("Загрузчик изображений"));
	TRC(IDOK, "Добавить");
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

bool ExtractLinks(CString text, std::vector<CString> &result)
{
	pcrepp::Pcre reg("((http|https|ftp)://[\\w\\d:#@%/;$()~_?\\+-=\\\\\\.&]*)", "imcu");
	std::string str = WCstringToUtf8(text);
	size_t pos = 0;
	while (pos <= str.length()) 
	{
		if( reg.search(str, pos)) 
		{ 
			pos = reg.get_match_end()+1;
			CString temp = Utf8ToWstring(reg[0]).c_str();
			result.push_back(temp);
		}
		else
			break;
	}
	return true;
}

LRESULT CImageDownloaderDlg::OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{	
	BeginDownloading();
	return 0;
}

LRESULT CImageDownloaderDlg::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
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

LRESULT CImageDownloaderDlg::OnChangeCbChain(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND hwndRemove = (HWND) wParam;  // handle of window being removed 
	HWND hwndNext = (HWND) lParam;

	if(hwndRemove == PrevClipboardViewer) PrevClipboardViewer = hwndNext;
	else ::SendMessage(PrevClipboardViewer, WM_CHANGECBCHAIN, wParam, lParam);
	return 0;
}

void CImageDownloaderDlg::OnDrawClipboard()
{
	bool IsClipboard = IsClipboardFormatAvailable(CF_TEXT)!=0;

	if(IsClipboard && SendDlgItemMessage(IDC_WATCHCLIPBOARD,BM_GETCHECK)==BST_CHECKED && !m_FileDownloader.IsRunning()	)
	{
		CString str;  
		IU_GetClipboardText(str);
		ParseBuffer(str, true);
		
	}
	//Sending WM_DRAWCLIPBOARD msg to the next window in the chain
	if(PrevClipboardViewer) ::SendMessage(PrevClipboardViewer, WM_DRAWCLIPBOARD, 0, 0); 
}

LRESULT CImageDownloaderDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ChangeClipboardChain(PrevClipboardViewer);
	return 0;
}

CString GetExtensionByMime(CString mime)
{
	TCHAR szImgTypes[3][4]={_T("jpg"),_T("png"),_T("gif")};
	TCHAR szMimeTypes[3][12]={_T("jpeg"),_T("png"),_T("gif")};
	for(int i=0;i<3;i++)
	{
		if(mime.Find(szMimeTypes[i])>=0)
			return szImgTypes[i];
	}
	return _T(".dat");
}

bool CImageDownloaderDlg::OnFileFinished(bool ok, CFileDownloader::DownloadFileListItem it)
{
	if(ok)
	{
		AddImageStruct ais;
		ais.show =true;
		ais.RealFileName = Utf8ToWstring(it.fileName).c_str();
		ais.VirtualFileName =  Utf8ToWstring(it.displayName).c_str();
		bool add = true;
		if(!IsImage(ais.RealFileName))
		{
			CString mimeType = Utf8ToWCstring(IuCoreUtils::GetFileMimeType(WCstringToUtf8(ais.RealFileName)));
			if(mimeType.Find(_T("image/"))>=0)
			{
				CString ext = GetExtensionByMime(mimeType);
				if(!ext.IsEmpty())
				{
					CString newFileName = ais.RealFileName + _T(".")+ext;
					MoveFile(ais.RealFileName, newFileName);
					 ais.RealFileName = newFileName;
					ais.VirtualFileName+=_T(".")+ext;
				}
			}
			else 
			{
				add = false;
				CString errorStr;
				errorStr.Format(TR("Файл '%s' не является файлом изображения (Mime-Type: %s)."),(LPCTSTR)(Utf8ToWstring(it.url).c_str()),(LPCTSTR)mimeType);
				WriteLog(logWarning,_T("Image Downloader"),errorStr);
			}
		}
		if(add)
			SendMessage(m_WizardDlg->m_hWnd, WM_MY_ADDIMAGE,(WPARAM)&ais,  0);

	}
	m_nFileDownloaded++;
	SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETPOS,  m_nFileDownloaded);
	return true;
}

void CImageDownloaderDlg::OnQueueFinished()
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

bool CImageDownloaderDlg::BeginDownloading()
{
	int index=0;

	std::string links = WCstringToUtf8(IU_GetWindowText(GetDlgItem(IDC_FILEINFOEDIT)));
	std::vector<std::string> tokens;
	nm_splitString(links,"\n",tokens,-1);
	m_nFilesCount =0;
	m_nFileDownloaded = 0;
	for(size_t i=0; i<tokens.size(); i++)
	{
		m_FileDownloader.AddFile(nm_trimStr(tokens[i]), (void*)i);
		m_nFilesCount++;
	}
	if(m_nFilesCount)
	{
		TRC(IDCANCEL, "Отмена");
		::EnableWindow(GetDlgItem(IDOK),false);
		::EnableWindow(GetDlgItem(IDC_WATCHCLIPBOARD),false);
		::EnableWindow(GetDlgItem(IDC_FILEINFOEDIT),false);
		::ShowWindow(GetDlgItem(IDC_DOWNLOADFILESPROGRESS),SW_SHOW);
		SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, m_nFilesCount));
		SendDlgItemMessage(IDC_DOWNLOADFILESPROGRESS, PBM_SETPOS,  0);
		m_FileDownloader.onFileFinished.bind(this, &CImageDownloaderDlg::OnFileFinished);
		m_FileDownloader.onQueueFinished.bind(this, &CImageDownloaderDlg::OnQueueFinished);
		m_FileDownloader.onConfigureNetworkManager.bind(this, &CImageDownloaderDlg::OnConfigureNetworkManager);
		m_FileDownloader.start();
		return true;
	}
	return false;
}

bool CImageDownloaderDlg::LinksAvailableInText(const CString &text)
{
	std::vector<CString> links;
	ExtractLinks(text,links);
	return links.size()!=0;
}

void CImageDownloaderDlg::ParseBuffer(const CString& buffer,bool OnlyImages)
{
	std::vector<CString> links;
	ExtractLinks(buffer,links);
	CString text = IU_GetWindowText(GetDlgItem(IDC_FILEINFOEDIT));
	for(size_t i=0; i<links.size(); i++)
	{
		CString fileName = myExtractFileName(links[i]);
		if((!OnlyImages && CString(GetFileExt(fileName)).IsEmpty()) || IsImage(fileName))
			text+=links[i]+_T("\r\n");
	}
	SetDlgItemText(IDC_FILEINFOEDIT, text);
}

void CImageDownloaderDlg::OnConfigureNetworkManager(NetworkManager* nm)
{
	IU_ConfigureProxy(*nm);
}