// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "MainDlg.h"
#include "resource.h"
#include "Func/Myutils.h"
#include "Core/Upload/Uploader.h"

#include "Func/Settings.h"
#include "Gui/Dialogs/LogWindow.h"
#include <GdiPlus.h>
#include "Gui/Dialogs/InputDialog.h"
#include "Core/Utils/CoreUtils.h"

const CString MyBytesToString(__int64 nBytes )
{
	return IuCoreUtils::fileSizeToString(nBytes).c_str();
}

CString IU_GetFileInfo(CString fileName,MyFileInfo* mfi=0)
{
	CString result;
	int fileSize = MyGetFileSize(fileName);
	result =  MyBytesToString(fileSize)+_T("(")+IntToStr(fileSize)+_T(");");
   CString mimeType = Utf8ToWCstring(IuCoreUtils::GetFileMimeType(WCstringToUtf8(fileName)));
	result+=mimeType+_T(";");
	if(mfi) mfi->mimeType = mimeType;
	if(mimeType.Find(_T("image/"))>=0)
	{
		Gdiplus::Image pic(fileName);
		int width = pic.GetWidth();
		int height = pic.GetHeight();
		if(mfi)
		{
			mfi->width = width;
			mfi->height = height;
		}
		result+= IntToStr(width)+_T("x")+IntToStr(height);
	}
	return result;
}



//#include "../Uploader.h"
LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_bIsRunning = false;
	
	CenterWindow(); // center the dialog on the screen
	DlgResize_Init(false, true, 0); // resizable dialog without "griper"

	CreateTempFolder();
	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);
	m_ListView = GetDlgItem(IDC_TOOLSERVERLIST);

	LogWindow.Create(0);
	LogWindow.Show();
	m_ListView.AddColumn(_T("N"), 0);
	m_ListView.AddColumn(_T("Server"), 1);
	m_ListView.AddColumn(_T("Status"), 2);
	m_ListView.AddColumn(_T("Direct URL"), 3);
	m_ListView.AddColumn(_T("Thumb URL"), 4);
	m_ListView.AddColumn(_T("View URL"), 5);
	m_ListView.AddColumn(_T("Time"), 6);
	m_ListView.SetColumnWidth(0, 30);
	m_ListView.SetColumnWidth(1, 80);
	m_ListView.SetColumnWidth(2, 120);
	m_ListView.SetColumnWidth(3, 240);
	m_ListView.SetColumnWidth(4, 240);
	m_ListView.SetColumnWidth(5, 240);
	m_ListView.SetColumnWidth(6, 50);

	SendDlgItemMessage(IDC_RADIOWITHACCS, BM_SETCHECK, 1);
	SendDlgItemMessage(IDC_CHECKIMAGESERVERS, BM_SETCHECK, 1);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	
	CString serversFileName = GetAppFolder() + "Data/" + _T("servers.xml");
	CString testFileName = GetAppFolder() + "testfile.jpg";
	if(xml.LoadFromFile( WCstringToUtf8((GetAppFolder() + "servertool.xml"))))
	{
		ZSimpleXmlNode root = xml.getRoot("ServerListTool");
		std::string name = root.Attribute("FileName");
		if(!name.empty())
			testFileName = Utf8ToWstring(name.c_str()).c_str();
	}
	
	SetDlgItemText(IDC_TOOLFILEEDIT, testFileName);
	Settings.LoadSettings();
	iuPluginManager.setScriptsDirectory(WstrToUtf8((LPCTSTR)(GetAppFolder() + "Data/Scripts/")));
	if(!m_ServerList.LoadFromFile(serversFileName))
	{
		MessageBox(_T("Cannot load server list!"));
	}

	m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 6);
	m_ListView.SetImageList(m_ImageList, LVSIL_NORMAL);
	for(int i=0; i<m_ServerList.count(); i++)
	{
		m_skipMap[i]=false;
		m_ListView.AddItem(i, 0, IntToStr(i), i);
		m_ListView.SetItemText(i, 1, Utf8ToWstring(m_ServerList.byIndex(i)->Name).c_str());
	}

	m_FileDownloader.onFileFinished.bind(this, &CMainDlg::OnFileFinished);
	return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSimpleDialog<IDD_ABOUTBOX, FALSE> dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_ListView.DeleteAllItems();
	for(int i=0; i<m_ServerList.count(); i++)
	{
		ServerData sd;
		ZeroMemory(&sd,sizeof(sd));
		

		m_CheckedServersMap[i]=sd;
		m_ListView.AddItem(i,0,IntToStr(i));
		m_ListView.SetItemText(i, 1, Utf8ToWstring( m_ServerList.byIndex(i)->Name).c_str());
		//m_ListView.AddItem(i+1,0,_T(""));
	}
	CString fileName = IU_GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
	m_srcFileHash = IU_md5_file(fileName);
	CString report = _T("Source file: ")+IU_GetFileInfo(fileName, &m_sourceFileInfo);
	SetDlgItemText(IDC_TOOLSOURCEFILE,report);
	::EnableWindow(GetDlgItem(IDOK),false);
	m_NeedStop= false;
	m_bIsRunning = true;
	Start(); // start working thread
	return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(isRunning())
	{
		stop();
	}
	else 
	{
		ZSimpleXml savexml;
		CString fileName = IU_GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
		ZSimpleXmlNode root = savexml.getRoot("ServerListTool");
		root.SetAttribute("FileName", WstrToUtf8((LPCTSTR)fileName));
		root.SetAttribute("Time", int(GetTickCount()));
		savexml.SaveToFile(WstrToUtf8((LPCTSTR)(GetAppFolder()+"servertool.xml")));
		EndDialog(wID);
	}
	return 0;
}

DWORD CMainDlg::Run()
{
	
	bool useAccounts = SendDlgItemMessage(IDC_RADIOWITHACCS,BM_GETCHECK) || SendDlgItemMessage(IDC_RADIOALWAYSACCS,BM_GETCHECK);
	bool onlyAccs = SendDlgItemMessage(IDC_RADIOALWAYSACCS,BM_GETCHECK) == BST_CHECKED;

	bool CheckImageServers = SendDlgItemMessage(IDC_CHECKIMAGESERVERS,BM_GETCHECK) == BST_CHECKED;
	bool CheckFileServers = SendDlgItemMessage(IDC_CHECKFILESERVERS,BM_GETCHECK) == BST_CHECKED;
	CString fileName = IU_GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
	if(!FileExists(fileName))
	{
			return -1;
	}

	for(int i=0; i<m_ServerList.count(); i++)
	{
		if(m_NeedStop) break;
		if(m_skipMap[i]) continue;
		CUploader uploader;
		
		
		uploader.setThumbnailWidth(160);
		uploader.onNeedStop.bind(this, &CMainDlg::OnNeedStop);
		//uploader.ShouldStop = &m_NeedStop;
		CUploadEngineData *ue = m_ServerList.byIndex(i);
		if(ue->ImageHost && !CheckImageServers)
			continue;
		if(!ue->ImageHost && !CheckFileServers)
			continue;

		ServerSettingsStruct  ss = Settings.ServersSettings[Utf8ToWstring(ue->Name).c_str()];
		if(!useAccounts)
		{
			ss.authData.DoAuth  = false;
		ss.authData.Login = "";
		}
		if((ue->NeedAuthorization==2 || (onlyAccs && ue->NeedAuthorization)) && ss.authData.Login.empty())	
		{
			m_ListView.SetItemText(i,2,CString(_T("No account is set")));
			continue;
		}
		if(onlyAccs && !ue->NeedAuthorization) 
		{
			m_ListView.SetItemText(i,2,CString(_T("skipped")));
			continue;
		}
		CAbstractUploadEngine * uploadEngine =  m_ServerList.getUploadEngine(ue);
		if(!uploadEngine)  
		{
			m_ListView.SetItemText(i,2,CString(_T("FAILED to create upload engine..")));
			continue;
		}
		uploadEngine->setServerSettings(ss);
		uploader.setUploadEngine(uploadEngine);
		
		m_ListView.SetItemText(i,2,CString(_T("Uploading file..")));
		DWORD startTime = GetTickCount();
		if(!uploader.UploadFile(WCstringToUtf8(fileName),WCstringToUtf8(myExtractFileName(fileName))))
		{
			MarkServer(i);
		}
		
		DWORD endTime = GetTickCount() - startTime;
		m_CheckedServersMap[i].timeElapsed = endTime;
		
		CString imgUrl = Utf8ToWstring(uploader.getDirectUrl()).c_str();
		CString thumbUrl = Utf8ToWstring(uploader.getThumbUrl()).c_str();
		CString viewUrl = Utf8ToWstring(uploader.getDownloadUrl()).c_str();
		int nFilesToCheck=0;
		if(!imgUrl.IsEmpty())
		{
			m_FileDownloader.AddFile(uploader.getDirectUrl(), reinterpret_cast<void*>(i*10));
			nFilesToCheck++;
			m_ListView.SetItemText(i,3,imgUrl);
			
		}
		else 
		{
			if(!ue->ImageUrlTemplate.empty())
			{
				if(!ue->UsingPlugin)
				m_CheckedServersMap[i].filesChecked++;
				m_ListView.SetItemText(i, 3, _T("<empty>"));
			}
		}

		if(!thumbUrl.IsEmpty())
		{

			nFilesToCheck++;
			m_FileDownloader.AddFile(uploader.getThumbUrl(),reinterpret_cast<void*>(i*10+1));
			m_ListView.SetItemText(i, 4, thumbUrl);
		}
		else 
		{
			
			if(!ue->ThumbUrlTemplate.empty())
			{
				if(!ue->UsingPlugin)
				m_CheckedServersMap[i].filesChecked++;
				m_ListView.SetItemText(i, 4, _T("<empty>"));
			}
		}
		
		if(!viewUrl.IsEmpty())
		{
			nFilesToCheck++;
			m_FileDownloader.AddFile(uploader.getDownloadUrl(), reinterpret_cast<void*>(i*10+2));
			m_ListView.SetItemText(i,5,viewUrl);
		}
		else 
		{
			
			if(!ue->DownloadUrlTemplate.empty())
			{
				if(!ue->UsingPlugin)
				m_CheckedServersMap[i].filesChecked++;
				m_ListView.SetItemText(i,5,_T("<empty>"));
			}

		}
		m_CheckedServersMap[i].fileToCheck = nFilesToCheck;
		m_FileDownloader.start();
		
		if(nFilesToCheck)
		m_ListView.SetItemText(i,2,CString(_T("Checking links")));
		else
			MarkServer(i);
	}
	
	::EnableWindow(GetDlgItem(IDOK),true);
	m_bIsRunning = false;
	return 0;
}

bool CMainDlg::OnFileFinished(bool ok, CFileDownloader::DownloadFileListItem it)
{
	int serverId = reinterpret_cast<int>(it.id) /10;
	int fileId = reinterpret_cast<int>(it.id) % 10;
	int columnIndex=-1;
	
		columnIndex = 3 +fileId;
	m_CheckedServersMap[serverId].filesChecked++;
	m_CheckedServersMap[serverId].fileToCheck--;
	CString fileName = Utf8ToWstring(it.fileName).c_str();
	
	if(!ok)
	{
		m_CheckedServersMap[serverId].stars[fileId] = 0;
		m_ListView.SetItemText(serverId,columnIndex, _T("Cannot download file"));
	}
	if(IU_md5_file(fileName)== m_srcFileHash)
	{
		if(fileId == 0)
		m_CheckedServersMap[serverId].stars[fileId] = 5;
		else m_CheckedServersMap[serverId].stars[fileId] = 4;
		m_ListView.SetItemText(serverId,columnIndex, _T("Identical file"));
	}
	else
	{
		MyFileInfo mfi;

		CString report =IU_GetFileInfo(fileName, &mfi);
		
      CString mimeType = Utf8ToWCstring(IuCoreUtils::GetFileMimeType(WCstringToUtf8((fileName))));
		if(fileId<2)
		{
			if(mimeType.Find(_T("image/"))>=0) 
			{
					if(fileId ==0 && (mfi.width!=m_sourceFileInfo.width || mfi.height!=m_sourceFileInfo.height))
						m_CheckedServersMap[serverId].stars[fileId]=0;
					else
				m_CheckedServersMap[serverId].stars[fileId] = fileId==0?4:5;
			}
			else m_CheckedServersMap[serverId].stars[fileId] = 0;
		} else m_CheckedServersMap[serverId].stars[fileId] = 5;

		m_ListView.SetItemText(serverId,columnIndex, report);
	}
	if(m_CheckedServersMap[serverId].fileToCheck==0)
	MarkServer(serverId);
	return 0;
}

void CMainDlg::MarkServer(int id)
{
	int sum = m_CheckedServersMap[id].stars[0]+m_CheckedServersMap[id].stars[1]+m_CheckedServersMap[id].stars[2];
	int mark=0;
	int count = m_CheckedServersMap[id].filesChecked;
	if(count) mark = sum/count;
	
	CString timeLabel;
	int endTime = m_CheckedServersMap[id].timeElapsed;
	timeLabel.Format(_T("%02d:%02d"),(int)(endTime/60000),(int)( endTime/1000%60) );
	m_ListView.SetItemText(id,2,timeLabel);

	CString strMark;
	if(mark == 5)
	{
		strMark = _T("EXCELLENT");
		m_CheckedServersMap[id].color = RGB(0,255,50);
		
	}
	else if (mark >= 4 )
	{
		strMark = _T("OK");
		m_CheckedServersMap[id].color = RGB(145,213,0);
		//m_ListView.SetItemText(id*2,2,CString());
	}
	else 
	{
		strMark = _T("FAILED");
		m_CheckedServersMap[id].color = RGB(198,0,0);
		//m_CheckedServersMap[id].failed = true;
	}

	m_ListView.SetItemText(id,2,strMark );
	m_ListView.SetItemText(id,6,timeLabel);
	m_ListView.RedrawItems(id,id);
	
}

LRESULT CMainDlg::OnSkip(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int nIndex = m_ListView.GetSelectedIndex();
	if(nIndex>=0)
	{
	
	m_skipMap[nIndex] = !m_skipMap[nIndex];
	if(m_skipMap[nIndex])
	m_ListView.SetItemText(nIndex,2,_T("<SKIP>"));
	else m_ListView.SetItemText(nIndex,2,_T(""));
	}
	return 0;
}

LRESULT CMainDlg::OnListViewNMCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW) pnmh;

	switch(lplvcd->nmcd.dwDrawStage) 
	{
		case CDDS_PREPAINT :
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT:
			if(m_CheckedServersMap[lplvcd->nmcd.dwItemSpec].color)
			lplvcd->clrTextBk = m_CheckedServersMap[lplvcd->nmcd.dwItemSpec].color;
	
		return CDRF_NEWFONT;
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			lplvcd->clrText = RGB(255,0,0);
			return CDRF_NEWFONT;    
	}
	return 0;
}

LRESULT CMainDlg::OnBrowseButton(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fd(true,0,0,4|2,0,m_hWnd);

	if(fd.DoModal() != IDOK || !fd.m_szFileName) return 0;

	SetDlgItemText(IDC_TOOLFILEEDIT,fd.m_szFileName);
	return 0;
}

void CMainDlg::stop()
{
	m_NeedStop = true;
	if(m_FileDownloader.IsRunning())
		m_FileDownloader.stop();
}

bool CMainDlg::isRunning()
{
	return m_bIsRunning || m_FileDownloader.IsRunning();
}

bool CMainDlg::OnNeedStop()
{
	return m_NeedStop;
}

const std::string Impl_AskUserCaptcha(NetworkManager *nm, const std::string& url)
{
	CString wFileName = GetUniqFileName(IUTempFolder+Utf8ToWstring("captcha").c_str());

	nm->setOutputFile(IuCoreUtils::WstringToUtf8((const TCHAR*)wFileName));
	if(!nm->doGet(url))
		return "";
	CInputDialog dlg(_T("Image Uploader"), TR("¬ведите текст с картинки:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()),wFileName);
	nm->setOutputFile("");
	if(dlg.DoModal()==IDOK)
		return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
	return "";
}