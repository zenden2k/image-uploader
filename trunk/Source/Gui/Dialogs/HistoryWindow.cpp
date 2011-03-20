/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>
	 
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
#include "HistoryWindow.h"
#include "../../LangClass.h"
#include "../../Settings.h"
#include "../../Func/HistoryManager.h"
#include "../../Func/Base.h"
#include "../../ResultsPanel.h"

// CHistoryWindow
CHistoryWindow::CHistoryWindow()
{
	m_historyReader = 0;
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
	TRC(IDCANCEL, "�������");
	TRC(IDC_SESSIONSCOUNTDESCR, "����� ������:");
	TRC(IDC_FILESCOUNTDESCR, "����� ������:");
	TRC(IDC_UPLOADTRAFFICDESCR, "����� �����:");
	SetWindowText(TR("������� ��������"));
	std::string fName = ZBase::get()->historyManager()->makeFileName();
	
	TRC(IDC_DOWNLOADTHUMBS, "��������� ��������� �� ���������");
	std::vector<CString> files;
	historyFolder = IU_GetDataFolder()+_T("\\History\\");
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
	
	BOOL bDummy;
	OnMonthChanged(0,0, 0,bDummy);
	m_treeView.SetFocus();
	return 0;  // Let the system set the focus
}

BOOL CHistoryWindow::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

LRESULT CHistoryWindow::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	::EnableWindow(GetDlgItem(IDCANCEL), false);
	m_treeView.abortLoadingThreads();

	EndDialog(0);
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
		menu.AppendMenu(MF_STRING, ID_OPENINBROWSER, TR("������� � ��������"));
		menu.AppendMenu(MF_STRING, ID_COPYTOCLIPBOARD, TR("���������� �����"));
	}
	menu.AppendMenu(MF_STRING, ID_VIEWBBCODE, TR("���� BBCode/HTML"));
	if(!isSessionItem)
	{
		std::string fileName  = historyItem->localFilePath;
		if(!isSessionItem && !fileName.empty() && IuCoreUtils::DirectoryExists(IuCoreUtils::ExtractFilePath(fileName)))
		{
			menu.AppendMenu(MF_STRING, ID_OPENFOLDER, TR("������� ����� � ������"));
		}
	}
	menu.SetMenuDefaultItem(0, true);
	menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ScreenPoint.x, ScreenPoint.y, m_hWnd);
	return 0;
}

void CHistoryWindow::FillList(CHistoryReader * mgr)
{
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
	return 0;
}
		
void CHistoryWindow::LoadHistoryFile(CString fileName)
{
	m_treeView.abortLoadingThreads();
	m_treeView.ResetContent();

	delete m_historyReader;
	m_historyReader = new CHistoryReader();
	m_historyReader->loadFromFile(WCstringToUtf8(historyFolder + fileName));
	FillList(m_historyReader);
}