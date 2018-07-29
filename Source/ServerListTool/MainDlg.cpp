// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "MainDlg.h"
#include "resource.h"
#include "Func/Myutils.h"
#include "Core/Upload/Uploader.h"

#include "Core/Settings.h"
#include "Gui/Dialogs/LogWindow.h"
#include "3rdpart/GdiPlusH.h"
#include "Gui/Dialogs/InputDialog.h"
#include "Core/Utils/CoreUtils.h"
#include "Gui/GuiTools.h"
#include "Func/IuCommonFunctions.h"
#include "Func/WinUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Core/CoreFunctions.h"
#include "Core/Upload/FileUploadTask.h"
#include "Core/Upload/UrlShorteningTask.h"
#include "Core/Upload/UploadSession.h"
#include "Core/Upload/UploadManager.h"

CString MyBytesToString(__int64 nBytes )
{
    return IuCoreUtils::fileSizeToString(nBytes).c_str();
}

CString IU_GetFileInfo(CString fileName,MyFileInfo* mfi=0)
{
    CString result;
    int fileSize = MyGetFileSize(fileName);
    result =  MyBytesToString(fileSize)+_T("(")+WinUtils::IntToStr(fileSize)+_T(" bytes);");
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
        result+= WinUtils::IntToStr(width)+_T("x")+WinUtils::IntToStr(height);
    }
    return result;
}

CMainDlg::CMainDlg(UploadEngineManager* uploadEngineManager, UploadManager* uploadManager, CMyEngineList* engineList) {
    uploadEngineManager_ = uploadEngineManager;
    uploadManager_ = uploadManager;
    engineList_ = engineList;
}

//#include "../Uploader.h"
LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    m_bIsRunning = false;
    
    CenterWindow(); // center the dialog on the screen
    DlgResize_Init(false, true, 0); // resizable dialog without "griper"

    IuCommonFunctions::CreateTempFolder();
    // set icons
    HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
        IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
    SetIcon(hIcon, TRUE);
    HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
        IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    SetIcon(hIconSmall, FALSE);
    m_ListView = GetDlgItem(IDC_TOOLSERVERLIST);

    //LogWindow.Create(0);
    m_ListView.AddColumn(_T("N"), 0);
    m_ListView.AddColumn(_T("Server"), 1);
    m_ListView.AddColumn(_T("Status"), 2);
    m_ListView.AddColumn(_T("Direct URL"), 3);
    m_ListView.AddColumn(_T("Thumb URL"), 4);
    m_ListView.AddColumn(_T("View URL"), 5);
    m_ListView.AddColumn(_T("Time"), 6);
    m_ListView.SetColumnWidth(0, 30);
    m_ListView.SetColumnWidth(1, 150);
    m_ListView.SetColumnWidth(2, 100);
    m_ListView.SetColumnWidth(3, 205);
    m_ListView.SetColumnWidth(4, 205);
    m_ListView.SetColumnWidth(5, 205);
    m_ListView.SetColumnWidth(6, 50);

    SendDlgItemMessage(IDC_RADIOWITHACCS, BM_SETCHECK, 1);
    SendDlgItemMessage(IDC_CHECKIMAGESERVERS, BM_SETCHECK, 1);
    m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
    

    CString testFileName = WinUtils::GetAppFolder() + "testfile.jpg";
    CString testURL = "https://github.com/zenden2k/image-uploader/issues/91";
    if(xml.LoadFromFile( WCstringToUtf8((WinUtils::GetAppFolder() + "servertool.xml"))))
    {
        SimpleXmlNode root = xml.getRoot("ServerListTool");
        std::string name = root.Attribute("FileName");
        if (!name.empty()) {
            testFileName = Utf8ToWstring(name.c_str()).c_str();
        }
        std::string url = root.Attribute("URL");
        if (!url.empty()) {
            testURL = Utf8ToWstring(url.c_str()).c_str();
        }
    }
    Settings.MaxThreads = 10;

    SetDlgItemText(IDC_TOOLFILEEDIT, testFileName);
    SetDlgItemText(IDC_TESTURLEDIT, testURL);
    Settings.LoadSettings("", "");
    Settings.ConnectionSettings.UseProxy = ConnectionSettingsStruct::kSystemProxy;
    //iuPluginManager.setScriptsDirectory(WstrToUtf8((LPCTSTR)(WinUtils::GetAppFolder() + "Data/Scripts/")));


    m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 6);
    m_ListView.SetImageList(m_ImageList, LVSIL_NORMAL);
    for (int i = 0; i < engineList_->count(); i++) {
        m_skipMap[i] = false;
        m_ListView.AddItem(i, 0, WinUtils::IntToStr(i), i);
        CString name = Utf8ToWstring(engineList_->byIndex(i)->Name).c_str();
        if (engineList_->byIndex(i)->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            name += _T("  [URL Shortener]");
        }
        m_ListView.SetItemText(i, 1, name);
    }

    m_FileDownloader.onConfigureNetworkClient.bind(this, &CMainDlg::OnConfigureNetworkClient);
    m_FileDownloader.onFileFinished.bind(this, &CMainDlg::OnFileFinished);
    return TRUE;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CSimpleDialog<IDD_ABOUTBOX, TRUE> dlg;
    dlg.DoModal();
    return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
    if (!WinUtils::FileExists(fileName)) {
        LOG(ERROR) << "File not found " << fileName;
        return 0;
    }
    m_ListView.DeleteAllItems();
    for (int i = 0; i < engineList_->count(); i++) {
        ServerData sd;
        ZeroMemory(&sd, sizeof(sd));
        m_CheckedServersMap[i] = sd;
        m_ListView.AddItem(i, 0, WinUtils::IntToStr(i));
        CString name = Utf8ToWstring(engineList_->byIndex(i)->Name).c_str();
        if (engineList_->byIndex(i)->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            name += _T("  [URL Shortener]");
        }
        m_ListView.SetItemText(i, 1, name);
    }
   
    m_srcFileHash = U2W(IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(W2U(fileName)));
    CString report = _T("Source file: ") + IU_GetFileInfo(fileName, &m_sourceFileInfo);
    SetDlgItemText(IDC_TOOLSOURCEFILE, report);
    ::EnableWindow(GetDlgItem(IDOK), false);
    GuiTools::ShowDialogItem(m_hWnd, IDC_STOPBUTTON, true);
    m_NeedStop = false;
    m_bIsRunning = true;
    //Start(); // start working thread
    Run(); 
    return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (isRunning()) {
        stop();
    } else {
        SimpleXml savexml;
        CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
        SimpleXmlNode root = savexml.getRoot("ServerListTool");
        root.SetAttribute("FileName", WstrToUtf8((LPCTSTR)fileName));
        root.SetAttribute("Time", int(GetTickCount()));
        savexml.SaveToFile(WstrToUtf8((LPCTSTR)(WinUtils::GetAppFolder() + "servertool.xml")));
        EndDialog(wID);
    }
    return 0;
}

DWORD CMainDlg::Run() {
    bool useAccounts = SendDlgItemMessage(IDC_RADIOWITHACCS, BM_GETCHECK) || SendDlgItemMessage(IDC_RADIOALWAYSACCS, BM_GETCHECK);
    bool onlyAccs = SendDlgItemMessage(IDC_RADIOALWAYSACCS, BM_GETCHECK) == BST_CHECKED;

    bool CheckImageServers = SendDlgItemMessage(IDC_CHECKIMAGESERVERS, BM_GETCHECK) == BST_CHECKED;
    bool CheckFileServers = SendDlgItemMessage(IDC_CHECKFILESERVERS, BM_GETCHECK) == BST_CHECKED;
    bool CheckURLShorteners = SendDlgItemMessage(IDC_CHECKURLSHORTENERS, BM_GETCHECK) == BST_CHECKED;
    CString fileName = GuiTools::GetWindowText(GetDlgItem(IDC_TOOLFILEEDIT));
    if (!WinUtils::FileExists(fileName)) {
        LOG(ERROR) << "File not found " << fileName;
        processFinished();
        return -1;
    }
    CString url = GuiTools::GetWindowText(GetDlgItem(IDC_TESTURLEDIT));
    if (CheckURLShorteners && url.IsEmpty()) {
        LOG(ERROR) << "URL should not be empty!";
        processFinished();
        return -1;
    }
    uploadSession_.reset(new UploadSession());
    uploadSession_->addSessionFinishedCallback(UploadSession::SessionFinishedCallback(this, &CMainDlg::onSessionFinished));
    int taskCount = 0;
    for (int i = 0; i < engineList_->count(); i++) {
        if (m_NeedStop) break;
        if (m_skipMap[i]) continue;

        //uploader.ShouldStop = &m_NeedStop;
        CUploadEngineData *ue = engineList_->byIndex(i);
        if (!(ue->hasType(CUploadEngineData::TypeImageServer) && CheckImageServers) &&
            !(ue->hasType(CUploadEngineData::TypeFileServer) && CheckFileServers)  &&
            !(ue->hasType(CUploadEngineData::TypeUrlShorteningServer) && CheckURLShorteners) ) {
            continue;
        }
            
        ServerSettingsStruct  ss;
        std::map <std::string, ServerSettingsStruct>::iterator it = Settings.ServersSettings[ue->Name].begin();

        if (!useAccounts) {
            if (it != Settings.ServersSettings[ue->Name].end()) {
                ss = it->second;
            }
        } else {
            if (it != Settings.ServersSettings[ue->Name].end()) {
                if (it->first.empty()) {
                    ++it;
                    if (it != Settings.ServersSettings[ue->Name].end()) {
                        ss = it->second;
                    }
                } else {
                    ss = it->second;
                }
            }
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
        ServerProfile serverProfile(ue->Name);
        serverProfile.setShortenLinks(false);
        serverProfile.setProfileName(ss.authData.Login);
        std::shared_ptr<UploadTask>  task;
        if (ue->hasType(CUploadEngineData::TypeImageServer) || ue->hasType(CUploadEngineData::TypeFileServer)) {
            task.reset(new FileUploadTask(WCstringToUtf8(fileName), WCstringToUtf8(myExtractFileName(fileName))));
            
        } else if (ue->hasType(CUploadEngineData::TypeUrlShorteningServer)) {
            task.reset(new UrlShorteningTask(WCstringToUtf8(url)));
        }

        task->setServerProfile(serverProfile);
        task->OnStatusChanged.bind(this, &CMainDlg::onTaskStatusChanged);
        task->addTaskFinishedCallback(UploadTask::TaskFinishedCallback(this, &CMainDlg::onTaskFinished));
        UploadTaskUserData* userData = new UploadTaskUserData();
        userData->rowIndex = i;

        task->setUserData(userData);
        uploadSession_->addTask(task);
        taskCount++;

        /*if (i > 3) {
            break;
        }*/
       // break;
    }
    if (taskCount) {
        uploadManager_->addSession(uploadSession_);
        uploadManager_->start();
    } else {
        processFinished();
    }
    
    return 0;
}

bool CMainDlg::OnFileFinished(bool ok, int statusCode, CFileDownloader::DownloadFileListItem it)
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
    if(IuCoreUtils::CryptoUtils::CalcMD5HashFromFile(W2U(fileName))== W2U(m_srcFileHash))
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
    int nIndex = -1;
    do {
        nIndex = m_ListView.GetNextItem(nIndex, LVNI_SELECTED);
        if (nIndex == -1) break;
        m_skipMap[nIndex] = !m_skipMap[nIndex];
        if (m_skipMap[nIndex])
            m_ListView.SetItemText(nIndex, 2, _T("<SKIP>"));
        else m_ListView.SetItemText(nIndex, 2, _T(""));

    } while (nIndex != -1);

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
    if(m_FileDownloader.isRunning())
        m_FileDownloader.stop();
}

bool CMainDlg::isRunning()
{
    return m_bIsRunning || m_FileDownloader.isRunning();
}

bool CMainDlg::OnNeedStop()
{
    return m_NeedStop;
}

void CMainDlg::processFinished() {
    ::EnableWindow(GetDlgItem(IDOK), true);
    m_bIsRunning = false;
    GuiTools::ShowDialogItem(m_hWnd, IDC_STOPBUTTON, false);
}

void CMainDlg::checkShortUrl(UploadTask* task) {
    NetworkClient client;
    CoreFunctions::ConfigureProxy(&client);
    UrlShorteningTask* urlTask = dynamic_cast<UrlShorteningTask*>(task);
    UploadTaskUserData* userData = reinterpret_cast<UploadTaskUserData*>(task->userData());
    client.setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
    
    bool ok = false;
    std::string targetUrl = task->uploadResult()->directUrl;
    if (!targetUrl.empty()) {
        int responseCode = 0;
        do {
            client.setCurlOptionInt(CURLOPT_FOLLOWLOCATION, 0);
            client.doGet(targetUrl);
            responseCode = client.responseCode();
            targetUrl = client.responseHeaderByName("Location");
            
        } while (!targetUrl.empty() && (responseCode == 302 || responseCode == 301) && targetUrl != urlTask->getUrl());

        if (!targetUrl.empty() && targetUrl == urlTask->getUrl()) {
            m_ListView.SetItemText(userData->rowIndex, 3, _T("Good link"));
            ok = true;
        }
    }

    m_CheckedServersMap[userData->rowIndex].filesChecked++;
    m_CheckedServersMap[userData->rowIndex].stars[0] = ok?5:0;
    MarkServer(userData->rowIndex);

}

void CMainDlg::OnConfigureNetworkClient(NetworkClient* nm)
{
    CoreFunctions::ConfigureProxy(nm);
}
void CMainDlg::OnConfigureNetworkClient(CUploader* uploader, NetworkClient* nm)
{
    CoreFunctions::ConfigureProxy(nm);
}

void CMainDlg::onTaskStatusChanged(UploadTask* task) {
    CUploadEngineData* ue = task->serverProfile().uploadEngineData();
    UploadTaskUserData* userData = reinterpret_cast<UploadTaskUserData*>(task->userData());
    int i = userData->rowIndex;
    if (task->status() == UploadTask::StatusRunning) {
        userData->startTime = GetTickCount();

        m_ListView.SetItemText(i, 2, CString(task->type() == UploadTask::TypeUrl ? _T("Shortening link.."):_T("Uploading file..")));
    } else if (task->status() == UploadTask::StatusFinished) {
       
    } else if (task->status() == UploadTask::StatusFailure) {
        
    }
}

void CMainDlg::onTaskFinished(UploadTask* task, bool ok) {
    CUploadEngineData* ue = task->serverProfile().uploadEngineData();
    UploadTaskUserData* userData = reinterpret_cast<UploadTaskUserData*>(task->userData());
    int i = userData->rowIndex;
    if (task->status() == UploadTask::StatusStopped) {
        m_ListView.SetItemText(i, 2, CString(_T("")));
        return;
    }
    if (ok) {
        DWORD endTime = GetTickCount() - userData->startTime;
        UploadResult* result = task->uploadResult();
        m_CheckedServersMap[i].timeElapsed = endTime;

        if (task->type() == UploadTask::TypeFile) {
            CString imgUrl = Utf8ToWstring(result->getDirectUrl()).c_str();
            CString thumbUrl = Utf8ToWstring(result->getThumbUrl()).c_str();
            CString viewUrl = Utf8ToWstring(result->getDownloadUrl()).c_str();
            int nFilesToCheck = 0;
            if (!imgUrl.IsEmpty()) {
                m_FileDownloader.addFile(result->getDirectUrl(), reinterpret_cast<void*>(i * 10));
                nFilesToCheck++;
                m_ListView.SetItemText(i, 3, imgUrl);

            } else {
                if (!ue->ImageUrlTemplate.empty()) {
                    if (!ue->UsingPlugin)
                        m_CheckedServersMap[i].filesChecked++;
                    m_ListView.SetItemText(i, 3, _T("<empty>"));
                }
            }

            if (!thumbUrl.IsEmpty()) {

                nFilesToCheck++;
                m_FileDownloader.addFile(result->getThumbUrl(), reinterpret_cast<void*>(i * 10 + 1));
                m_ListView.SetItemText(i, 4, thumbUrl);
            } else {

                if (!ue->ThumbUrlTemplate.empty()) {
                    if (!ue->UsingPlugin)
                        m_CheckedServersMap[i].filesChecked++;
                    m_ListView.SetItemText(i, 4, _T("<empty>"));
                }
            }

            if (!viewUrl.IsEmpty()) {
                nFilesToCheck++;
                m_FileDownloader.addFile(result->getDownloadUrl(), reinterpret_cast<void*>(i * 10 + 2));
                m_ListView.SetItemText(i, 5, viewUrl);
            } else {

                if (!ue->DownloadUrlTemplate.empty()) {
                    if (!ue->UsingPlugin)
                        m_CheckedServersMap[i].filesChecked++;
                    m_ListView.SetItemText(i, 5, _T("<empty>"));
                }

            }
            m_CheckedServersMap[i].fileToCheck = nFilesToCheck;
            m_FileDownloader.start();
            if (nFilesToCheck)
                m_ListView.SetItemText(i, 2, CString(_T("Checking links")));
            else
                MarkServer(i);
        } else if (task->type() == UploadTask::TypeUrl) {
            CString shortURL = Utf8ToWstring(result->getDirectUrl()).c_str();
            if (!shortURL.IsEmpty()) {
                m_ListView.SetItemText(i, 3, shortURL);
                m_ListView.SetItemText(i, 2, CString(_T("Checking short link")));
                checkShortUrl(task);
            } else {
                m_ListView.SetItemText(i, 3, _T("<empty>"));
            }
        }
        
        
    } else {
        MarkServer(i);
    }
}

void CMainDlg::onSessionFinished(UploadSession* session) {
    processFinished();
    LOG(WARNING) << "Uploader has finished";
}

LRESULT CMainDlg::OnErrorLogButtonClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    LogWindow.ShowWindow(LogWindow.IsWindowVisible()? SW_HIDE:  SW_SHOW);
    return 0;
}

LRESULT CMainDlg::OnSkipAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    for(int i = 0; i < engineList_->count(); i++)
    {
        m_skipMap[i] = true;
        m_ListView.SetItemText(i, 2, _T("<SKIP>"));
    }
    return 0;
}


LRESULT CMainDlg::OnBnClickedStopbutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    uploadSession_->stop();
    m_FileDownloader.stop();
    return 0;
}
