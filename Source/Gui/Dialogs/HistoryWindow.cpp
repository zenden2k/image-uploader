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
#include "../../atlheaders.h"
#include "HistoryWindow.h"
#include "../../Func/LangClass.h"
#include "../../Func/Settings.h"
#include "../../Func/HistoryManager.h"
#include "../../Func/Base.h"
#include "ResultsPanel.h"

// CHistoryWindow
CHistoryWindow::CHistoryWindow()
{
	m_historyReader = 0;
	delayed_closing_ = false;
}

CHistoryWindow::~CHistoryWindow()
{
	if(m_hWnd) 
	{
		Detach();
		m_hWnd = NULL;
	}
	delete m_historyReader;
}

LRESULT CHistoryWindow::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow();
	DlgResize_Init();
	m_treeView.SubclassWindow(GetDlgItem(IDC_HISTORYTREE));
	m_treeView.onThreadsFinished.bind(this, &CHistoryWindow::threadsFinished);
	m_treeView.onThreadsStarted.bind(this, &CHistoryWindow::threadsStarted);
	TRC(IDCANCEL, "Закрыть");
	TRC(IDC_SESSIONSCOUNTDESCR, "Всего сессий:");
	TRC(IDC_FILESCOUNTDESCR, "Всего файлов:");
	TRC(IDC_UPLOADTRAFFICDESCR, "Общий объем:");
	SetWindowText(TR("История загрузок"));
	TRC(IDC_TIMEPERIODLABEL, "Период времени:");
	TRC(IDC_DOWNLOADTHUMBS, "Загружать миниатюры из Интернета");

	HWND hWnd = GetDlgItem(IDC_ANIMATIONSTATIC);
	if (hWnd)
	{
		m_wndAnimation.SubclassWindow(hWnd);
		if (m_wndAnimation.Load(MAKEINTRESOURCE(IDR_PROGRESSGIF),_T("GIF")))
			m_wndAnimation.Draw();
		m_wndAnimation.ShowWindow(SW_HIDE);
	};

	std::string fName = ZBase::get()->historyManager()->makeFileName();
	
	std::vector<CString> files;
	historyFolder = Settings.SettingsFolder+_T("\\History\\");
	GetFolderFileList(files, historyFolder , _T("history*.xml"));
	for(size_t i=0; i<files.size(); i++)
	{
		m_HistoryFiles.push_back(files[i]);

		CString monthLabel = Utf8ToWCstring( IuCoreUtils::ExtractFileNameNoExt(WCstringToUtf8 (files[i])));
		monthLabel.Replace(_T("history_"), _T(""));
		monthLabel.Replace(_T("_"), _T("/"));
		SendDlgItemMessage(IDC_MONTHCOMBO, CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)monthLabel);
	}
	int selectedIndex = files.size()-1;
	SendDlgItemMessage(IDC_MONTHCOMBO, CB_SETCURSEL, selectedIndex, 0);
	
	SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_SETCHECK, (WPARAM)Settings.HistorySettings.EnableDownloading);
	BOOL bDummy;
	OnMonthChanged(0,0, 0,bDummy);
	m_treeView.SetFocus();
	return 1;  // Let the system set the focus
}

BOOL CHistoryWindow::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CHistoryWindow::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	delayed_closing_ = true;
	if(!m_treeView.isRunning())
	{
		
		
		Settings.HistorySettings. EnableDownloading = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
	
		EndDialog(0);
	}
	else
	{
		::EnableWindow(GetDlgItem(IDCANCEL), false);
		::EnableWindow(GetDlgItem(IDC_MONTHCOMBO), true);
		m_treeView.EnableWindow(true);
		m_treeView.abortLoadingThreads();
	}
	return 0;
}

void CHistoryWindow::Show()
{
	if(!IsWindowVisible())
			ShowWindow(SW_SHOW);
	SetForegroundWindow(m_hWnd);
}

LRESULT CHistoryWindow::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND 	hwnd = (HWND) wParam;  
	POINT ClientPoint, ScreenPoint;
	if(hwnd != GetDlgItem(IDC_HISTORYTREE)) return 0;

	if(lParam == -1) 
	{
		ClientPoint.x = 0;
		ClientPoint.y = 0;
		ScreenPoint = ClientPoint;
		::ClientToScreen(hwnd, &ScreenPoint);
	}
	else
	{
		ScreenPoint.x = LOWORD(lParam); 
		ScreenPoint.y = HIWORD(lParam); 
		ClientPoint = ScreenPoint;
		::ScreenToClient(hwnd, &ClientPoint);
	}
	bool isSessionItem  = false;
	TreeItem* item = m_treeView.selectedItem();
	if(!item) return 0;
	
	 isSessionItem = item->level()==0;
	
	 HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
	//if(!m_treeView.IsItemAtPos(ClientPoint.x, ClientPoint.y, isSessionItem)) return 0;
	CMenu menu;
	menu.CreatePopupMenu();
	if(!isSessionItem)
	{
		menu.AppendMenu(MF_STRING, ID_OPENINBROWSER, TR("Открыть в браузере"));
		menu.AppendMenu(MF_STRING, ID_COPYTOCLIPBOARD, TR("Копировать адрес"));
	}
	menu.AppendMenu(MF_STRING, ID_VIEWBBCODE, TR("Коды BBCode/HTML"));
	if(!isSessionItem)
	{
		std::string fileName  = historyItem->localFilePath;
		if(!isSessionItem && !fileName.empty() && IuCoreUtils::DirectoryExists(IuCoreUtils::ExtractFilePath(fileName)))
		{
			menu.AppendMenu(MF_STRING, ID_OPENFOLDER, TR("Открыть папку с файлом"));
		}
	}
	menu.SetMenuDefaultItem(0, true);
	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	return 0;
}

void CHistoryWindow::FillList(CHistoryReader * mgr)
{
	bool enabledDownload = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
	m_treeView.setDownloadingEnabled(enabledDownload);
	int nSessionsCount = mgr->getSessionCount();

	m_treeView.SetRedraw(false);
	TreeItem* res = 0;
	int nFilesCount = 0;
	zint64 totalFileSize = 0;
	for(int i=0; i<nSessionsCount; i++)
	{
		 CHistorySession* ses = mgr->getSession(i);
		std::string serverName = ses->serverName();
		if(serverName.empty()) serverName = "n/a";

		std::string label = IuCoreUtils::timeStampToString(ses->timeStamp())+ "\r\n Server: "+ serverName+ " Files: " + IuCoreUtils::toString(ses->entriesCount()); 
		res = m_treeView.addEntry(ses, Utf8ToWCstring(label));
		int nCount = ses->entriesCount();
		for(int j=0; j<nCount; j++)
		{
			nFilesCount++;
			totalFileSize += ses->entry(j).uploadFileSize;
			m_treeView.addSubEntry(res, ses->entry(j),nCount<4);
		}
		//m_treeView.ExpandItem(res);
	}
	if(res)
	{
		m_treeView.SetCurSel(m_treeView.GetCount()-1);
		m_treeView.SetCurSel(-1);
	}
	m_treeView.SetRedraw(true);

	SetDlgItemInt(IDC_FILESCOUNTLABEL, nFilesCount, false);
	SetDlgItemInt(IDC_SESSIONSCOUNTLABEL, nSessionsCount, false);
	SetDlgItemText(IDC_UPLOADTRAFFICLABEL, Utf8ToWCstring(IuCoreUtils::fileSizeToString(totalFileSize)));
}

LRESULT CHistoryWindow::OnHistoryTreeCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	bHandled = true;
	if(m_treeView.m_hWnd == 0) return 0;
	return 0;
}

LRESULT CHistoryWindow::OnOpenInBrowser(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TreeItem* item = m_treeView.selectedItem();
	if(!item) return 0;
	HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
	std::string url = historyItem->directUrl.length()?historyItem->directUrl:historyItem->viewUrl;
	 ShellExecute(NULL, _T("open"), Utf8ToWCstring(url), NULL, NULL, SW_SHOWNORMAL);
	return 0;
}

LRESULT CHistoryWindow::OnCopyToClipboard(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TreeItem* item = m_treeView.selectedItem();
	if(!item) return 0;
	HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
	std::string url = historyItem->directUrl.length()?historyItem->directUrl:historyItem->viewUrl;
	IU_CopyTextToClipboard(Utf8ToWCstring(url));
	return 0;
}

CUrlListItem fromHistoryItem(HistoryItem historyItem)
{
	CUrlListItem it;
	it.ImageUrl = Utf8ToWstring(historyItem.directUrl).c_str();
	it.ThumbUrl =  Utf8ToWstring(historyItem.thumbUrl).c_str();
	it.DownloadUrl = Utf8ToWstring(historyItem.viewUrl).c_str();
	return it;
}
LRESULT CHistoryWindow::OnViewBBCode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TreeItem* item = m_treeView.selectedItem();
	if(!item) return 0;
	CAtlArray<CUrlListItem> items;

	if(item->level()==0)
	{
		CHistorySession* ses = reinterpret_cast<CHistorySession*>(item->userData());
		for(int i=0; i<ses->entriesCount(); i++)
		{
			CUrlListItem it  =fromHistoryItem(ses->entry(i));
			items.Add(it);
		}
	}
	else
	{
		HistoryItem* hit = reinterpret_cast<HistoryItem*>(item->userData());
		CUrlListItem it  = fromHistoryItem(*hit);
		items.Add(it);
	}
	CResultsWindow rp( pWizardDlg, items,false);
	rp.DoModal(m_hWnd);
	return 0;
}

LRESULT CHistoryWindow::OnOpenFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	TreeItem* item = m_treeView.selectedItem();
	if(!item) return 0;
	HistoryItem* historyItem = reinterpret_cast<HistoryItem*>(item->userData());
	std::string fileName  = historyItem->localFilePath;
	if(fileName.empty()) return 0;
	std::string directory = IuCoreUtils::ExtractFilePath(fileName);
	if(IuCoreUtils::FileExists(fileName))
	{
		ShellExecuteW(NULL, NULL, L"explorer.exe", CString(_T("/select, ")) + Utf8ToWCstring(fileName), NULL, SW_SHOWNORMAL);
	}
	else if(IuCoreUtils::DirectoryExists(directory))
	{
		ShellExecute(NULL, _T("open"), Utf8ToWCstring(directory), NULL, NULL, SW_SHOWNORMAL);
	}
	return 0;
}

LRESULT CHistoryWindow::OnMonthChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	int nIndex = SendDlgItemMessage(IDC_MONTHCOMBO, CB_GETCURSEL);
	if(nIndex == -1) return 0;
	LoadHistoryFile(m_HistoryFiles[nIndex]);
	m_treeView.SetFocus();
	return 0;
}
		
void CHistoryWindow::LoadHistoryFile(CString fileName)
{
	m_delayedFileName = fileName;
	if(!m_treeView.isRunning())
	{
		
		m_treeView.ResetContent();

		delete m_historyReader;
		m_historyReader = new CHistoryReader();
		m_historyReader->loadFromFile(WCstringToUtf8(historyFolder + fileName));
		FillList(m_historyReader);
		m_delayedFileName = "";
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_MONTHCOMBO), false);
		::EnableWindow(GetDlgItem(IDCANCEL), false);
		m_treeView.EnableWindow(false);
		m_treeView.abortLoadingThreads();
	}
}

void CHistoryWindow::threadsFinished()
{
	m_wndAnimation.ShowWindow(SW_HIDE);
	
	if(!m_delayedFileName.IsEmpty())
	{
		SendMessage(WM_MY_OPENHISTORYFILE, (WPARAM)(LPCTSTR)m_delayedFileName);
		//LoadHistoryFile(m_delayedFileName);
		
	}
	else if(delayed_closing_)
	{
		EndDialog(0);
		return;
	}
	::EnableWindow(GetDlgItem(IDC_MONTHCOMBO), true);
	m_treeView.EnableWindow(true);
	::EnableWindow(GetDlgItem(IDCANCEL), true);
}

void CHistoryWindow::threadsStarted()
{
	m_wndAnimation.ShowWindow(SW_SHOW);
}

LRESULT CHistoryWindow::OnDownloadThumbsCheckboxChecked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	Settings.HistorySettings.EnableDownloading = SendDlgItemMessage(IDC_DOWNLOADTHUMBS, BM_GETCHECK) == BST_CHECKED;
	m_treeView.setDownloadingEnabled(Settings.HistorySettings.EnableDownloading);
	return 0;
}
		
LRESULT CHistoryWindow::OnWmOpenHistoryFile(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LPCTSTR fileName = reinterpret_cast<TCHAR*>(wParam);
	//m_treeView.setDownloadingEnabled(false);
	LoadHistoryFile(fileName);
	return 0;
}