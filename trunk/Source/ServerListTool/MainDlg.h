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

// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include <map>
#include "resource.h"
#include "3rdpart/thread.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/FileDownloader.h"
#include "Func/MyEngineList.h"
#include "Core/Utils/SimpleXml.h"


struct ServerData
{
	int stars[3];
	COLORREF color;
	int fileToCheck;
	int filesChecked;
	int timeElapsed;
};

struct MyFileInfo
{
	int width;
	int height;
	CString mimeType;
	int size;
};
class CMainDlg :
	public CDialogImpl<CMainDlg>, public CThreadImpl<CMainDlg>, 
	 public  CDialogResize <CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDC_BUTTONSKIP, OnSkip)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_TOOLBROWSEBUTTON, OnBrowseButton)
		NOTIFY_HANDLER(IDC_TOOLSERVERLIST, NM_CUSTOMDRAW, OnListViewNMCustomDraw)
		CHAIN_MSG_MAP(CDialogResize<CMainDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CMainDlg)
		DLGRESIZE_CONTROL(IDC_TOOLSERVERLIST, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(ID_APP_ABOUT, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_BUTTONSKIP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSkip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBrowseButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CListViewCtrl m_ListView;
	CMyEngineList m_ServerList;
	DWORD Run();
	void stop();
	bool m_NeedStop;
	bool m_bIsRunning;
	bool isRunning();
	std::map<int,ServerData> m_CheckedServersMap;
	std::map<int, bool> m_skipMap;
	CImageList m_ImageList;
	CString m_srcFileHash;
	MyFileInfo m_sourceFileInfo;
	virtual bool OnFileFinished(bool ok,  CFileDownloader::DownloadFileListItem it);
	void MarkServer(int id);
	CFileDownloader m_FileDownloader;
	LRESULT OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	bool OnNeedStop();

	ZSimpleXml xml;
};
