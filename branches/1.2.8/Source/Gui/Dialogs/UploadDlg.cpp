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

#include "UploadDlg.h"

#include <shobjidl.h>
#include "Core/ImageConverter.h"
#include "Func/Base.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/InputDialog.h"
#include "Func/Settings.h"
#include "Core/Upload/UploadEngine.h"
#include "mediainfodlg.h"
#include <Gui/GuiTools.h>
#include <Func/WinUtils.h>
#include <Func/Myutils.h>
#include <Func/Common.h>

class CTempFilesDeleter
{
	public:
		CTempFilesDeleter();
		void AddFile(const CString& fileName);
		bool Cleanup();

	protected:
		std::vector<CString> m_files;
};

CTempFilesDeleter::CTempFilesDeleter()
{

}

void CTempFilesDeleter::AddFile(const CString& fileName)
{
	m_files.push_back(fileName);
}

bool CTempFilesDeleter::Cleanup()
{
	for(size_t i=0; i<m_files.size(); i++)
	{
		DeleteFile(m_files[i]);
	}
	m_files.clear();
	return true;
}


CString UploaderStatusToString(StatusType status, int actionIndex, std::string param)
{
	CString result;
	switch(status)
	{
		case stWaitingAnswer:
			result = TR("Ожидание ответа от сервера...");
			break;
		case stCreatingFolder:
			result = CString(TR("Создание папки \"")) + IuCoreUtils::Utf8ToWstring(param).c_str() + _T("\"...");
			break;
		case stUploading:
			result = TR("Отправка файла на сервер...");
			break;
		case stAuthorization:
			result = TR("Авторизация на сервере...");
			break;
		case stPerformingAction:
			result.Format(TR("Выполняю действие #%d..."), actionIndex);
			break;
		case stUserDescription:
			result = IuCoreUtils::Utf8ToWstring(param).c_str();
	}
	return result;
};


bool BytesToString(__int64 nBytes, LPTSTR szBuffer,int nBufSize)
{
	return WinUtils::NewBytesToString(nBytes, szBuffer, nBufSize);
}

// CUploadDlg
CUploadDlg::CUploadDlg(CWizardDlg *dlg):ResultsWindow(new CResultsWindow(dlg,UrlList,true))
{
	MainDlg = NULL;
	TimerInc = 0;
	IsStopTimer = false;
	Terminated = false;
	m_EngineList = _EngineList;
	LastUpdate = 0;
	#if  WINVER	>= 0x0601
		ptl = NULL;
	#endif
	queueUploader_.setCallback(this);
	queueUploader_.setMaxThreadCount( 4 );
	mutex_ = new ZThread::Mutex();
	tempFileDeleter_ = new CTempFilesDeleter();
	filesFinished_ = 0;
}

CUploadDlg::~CUploadDlg()
{
	delete ResultsWindow;
}

//Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);

LRESULT CUploadDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	createToolbar();

	uploadListView_.m_hWnd  = GetDlgItem(IDC_UPLOADTABLE);
	uploadListView_.AddColumn( TR("Файл"), 1);
	uploadListView_.AddColumn( TR("Статус"), 1);
	uploadListView_.AddColumn( TR("Миниатюра"), 2);
	uploadListView_.SetColumnWidth(0, 170);
	uploadListView_.SetColumnWidth(1, 170);
	uploadListView_.SetColumnWidth(2, 170);
	// Initializing Windows 7 taskbar related stuff
	RECT rc;
	::GetWindowRect(GetDlgItem(IDC_RESULTSPLACEHOLDER), &rc);
	::MapWindowPoints(0,m_hWnd, (POINT*)&rc, 2);
	ResultsWindow->Create(m_hWnd);
	ResultsWindow->SetWindowPos(0,&rc,0);
	#if  WINVER	>= 0x0601
		const GUID IID_ITaskbarList3 = { 0xea1afb91,0x9e28,0x4b86,{0x90,0xe9,0x9e,0x9f, 0x8a,0x5e,0xef,0xaf}};
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&ptl);
	#endif

	TRC(IDC_COMMONPROGRESS, "Общий прогресс:");
	bool IsLastVideo = lstrlen(MediaInfoDllPath)!=0;

	CVideoGrabber *vg = static_cast<CVideoGrabber*>(WizardDlg->Pages[1]);

	if(vg && lstrlen(vg->m_szFileName))
		IsLastVideo=true;

	EnableMediaInfo(IsLastVideo);
	
	SetDlgItemInt(IDC_THUMBSPERLINE, 4);
	SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
	
	GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPROGRESS));
	GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPERCENTS));
	PageWnd = m_hWnd;
	ResultsWindow->SetPage(Settings.CodeLang);
	ResultsWindow->SetCodeType(Settings.CodeType);
	currentTab_ = -1;
	showUploadProgressTab();
	return 1;  
}

#define Terminat() {Terminated=true; return 0;}

DWORD CUploadDlg::Run() {
	showUploadProgressTab();
	filesFinished_ = 0;
	m_bStopped = false;
	HRESULT hRes = ::CoInitialize(NULL);
	
	if ( !MainDlg ) {
		return 0;
	}


	int Server;
	int FileServer = Settings.FileServerID();
	int n = MainDlg->FileList.GetCount();

	if(Settings.QuickUpload && !WizardDlg->Pages[3])
		Server = Settings.QuickServerID();
	else Server = Settings.ServerID();

	if(Server == -1) {
		Server = m_EngineList->getRandomImageServer();
		if(Server == -1) {
			return ThreadTerminated();
		}
	}

	if(FileServer == -1)
	{
		FileServer = m_EngineList->getRandomFileServer();
		if(FileServer == -1) {
			return ThreadTerminated();
		}
	}

	CUploadEngineData *ue = m_EngineList->byIndex(Server);

#if  WINVER	>= 0x0601
	if(ptl)
		ptl->SetProgressState(GetParent(), TBPF_NORMAL); // initialise Windows 7 taskbar button progress 
#endif
	CHistoryManager * mgr = ZBase::get()->historyManager();
//	session_ = mgr->newSession();
	fileProcessingQueue_.clear();
	uploadListView_.DeleteAllItems();

	for ( int i = 0; i < n ; i++ ) {
		FileProcessingStruct fps;
		fps.fileName = WCstringToUtf8( MainDlg->FileList[i].FileName );
		fps.uploadEngineData = ue;
		fps.tableRow = i;
		fileProcessingQueue_.push_back(fps);
		uploadListView_.AddItem(i, 0, Utf8ToWCstring( IuCoreUtils::ExtractFileName(fps.fileName ) ));
		uploadListView_.AddItem(i, 1, TR("В очереди"));
	}
	/*m_CurrentUploader = &Uploader;
	Uploader.onNeedStop.bind(this, &CUploadDlg::OnUploaderNeedStop);
	Uploader.onProgress.bind(this, &CUploadDlg::OnUploaderProgress);
	Uploader.onDebugMessage.bind(DefaultErrorHandling::DebugMessage);
	Uploader.onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	Uploader.onStatusChanged.bind(this, &CUploadDlg::OnUploaderStatusChanged);
	Uploader.onConfigureNetworkManager.bind(this, &CUploadDlg::OnUploaderConfigureNetworkClient);*/

	addNewFilesToUploadQueue();
	
	/*int Errors = n-NumUploaded;
	if(Errors>0)
		wsprintf(szBuffer,CString(TR("Вcего %d файлов было загружено."))+_T(" ")
		+TR("%d файлов загружены не были из-за ошибок."),NumUploaded,Errors);
	else
		wsprintf(szBuffer,CString(TR("Вcего %d файлов было загружено."))+_T(" ")+TR("Ошибок нет."), NumUploaded );
	*/
	//Sleep(100000);
	return 0;
}

LRESULT CUploadDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 1) // This timer becomes enabled when user presses "Stop" button
	{
		TimerInc--;
		CString szBuffer;
		if(TimerInc>0)
		{
			szBuffer.Format(CString(TR("Остановить"))+_T(" (%d)"), TimerInc);
			SetNextCaption(szBuffer);
		}
		else
		{
			if(WizardDlg->IsWindowEnabled())
			{
				Terminate();
				ThreadTerminated();
				FileProgress(TR("Загрузка файлов прервана пользователем."), false);
			}
		}
	}

	else if(wParam == 2) // Another timer which updates status information int the window
	{

		PrInfo.CS.Lock();
		if(!PrInfo.ip.IsUploading) 
		{
			ShowProgress(false);
			if(m_CurrentUploader)
			{
				CString StatusText = m_StatusText;
				if(!StatusText.IsEmpty())
					FileProgress(StatusText);
			}
			PrInfo.CS.Unlock();
			return 0;
		}
		else if(PrInfo.ip.Total>0)
		{
			TCHAR buf1[100],buf2[100];
			BytesToString(PrInfo.ip.Uploaded,buf1,sizeof(buf1));
			BytesToString(PrInfo.ip.Total,buf2,sizeof(buf2));

			int perc = int(((float)PrInfo.ip.Uploaded/(float)PrInfo.ip.Total)*100);
			if(perc<0 || perc>100) perc=0;
			if(perc>=100) 
			{
				ShowProgress(false);
				PrInfo.Bytes.clear();
				PrInfo.ip.Uploaded=0;
				PrInfo.ip.Total = 0;
				PrInfo.ip.IsUploading  = false;
				PrInfo.CS.Unlock();
				return 0;
			}
			if(PrInfo.ip.IsUploading) 
			{
				if(!::IsWindowVisible(GetDlgItem(IDC_FILEPROGRESS)) /*&& PrInfo.Total==0*/)
				{
					::ShowWindow(GetDlgItem(IDC_FILEPROGRESS),SW_SHOW);
					::ShowWindow(GetDlgItem(IDC_SPEEDLABEL),SW_SHOW);
					::ShowWindow(GetDlgItem(IDC_PERCENTLABEL),SW_SHOW);
				}
			}
			TCHAR ProgressBuffer[256]=_T("");
			_stprintf (ProgressBuffer,TR("Загружено %s из %s"),buf1,buf2,(int)perc);
			FileProgress(ProgressBuffer);
			wsprintf (ProgressBuffer,_T ("%d %%"),(int)perc);
			SetDlgItemText(IDC_PERCENTLABEL,ProgressBuffer);
			SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,(int)perc);
//			UploadProgress(progressCurrent, progressTotal, perc/2);

			DWORD Current =  PrInfo.ip.Uploaded;
			TCHAR SpeedBuffer[256]=_T("\0");
			buf1[0]=0;


			int speed=0;
			if(PrInfo.Bytes.size())
			{
				speed= int(((double)(Current-/*LastUpdate*/PrInfo.Bytes[0])/(double)PrInfo.Bytes.size())*4);
			}
			PrInfo.Bytes.push_back(Current);
			if(PrInfo.Bytes.size()>11)
			{
				PrInfo.Bytes.pop_front(); //Deleting element at the beginning of the deque
			}
			if(speed>0)
			{
				BytesToString(speed,SpeedBuffer,sizeof(SpeedBuffer));
				wsprintf (buf1,_T ("%s/s"),SpeedBuffer);
				SetDlgItemText(IDC_SPEEDLABEL,buf1);
			}
			else SetDlgItemText(IDC_SPEEDLABEL,_T(""));


			LastUpdate = Current;

		}
		PrInfo.CS.Unlock();
	}
	return 0;
}

int CUploadDlg::ThreadTerminated(void) {
	ResultsWindow->FinishUpload();
//	FileProgress(szBuffer, false);

	WizardDlg->QuickUploadMarker = false;
	TCHAR szBuffer[MAX_PATH];
	SetDlgItemText(IDC_COMMONPROGRESS2, TR("Загрузка завершена."));
//	UploadProgress(MainDlg->FileList.GetCount(), MainDlg->FileList.GetCount());
	#if  WINVER	>= 0x0601
		if(ptl)
			ptl->SetProgressState(GetParent(), TBPF_NOPROGRESS);
	#endif
	wsprintf(szBuffer,_T("%d %%"),100);
	SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

	if(CancelByUser) {
		FileProgress(TR("Загрузка файлов прервана пользователем."), false);
	}
	else
	{

	}
	KillTimer(1);
	KillTimer(2);
	LastUpdate = 0;
	ShowProgress(false);

	SetNextCaption(TR("Завершить >"));
	Terminated = true;
	IsStopTimer = false;
	EnablePrev();
	EnableNext();
	EnableExit();
	return 0;
}

bool CUploadDlg::OnShow()
{
	bool IsLastVideo=false;

	if(lstrlen(MediaInfoDllPath))
	{
		CVideoGrabber *vg =(	CVideoGrabber *) WizardDlg->Pages[1];

		if(vg && lstrlen(vg->m_szFileName))
			IsLastVideo=true;
	}
	ResultsWindow->InitUpload();
	EnableMediaInfo(IsLastVideo);
	CancelByUser = false;
	ShowNext();
	ShowPrev();
	MainDlg = (CMainDlg*) WizardDlg->Pages[2];
	//Toolbar.CheckButton(IDC_USETEMPLATE,Settings.UseTxtTemplate);
	FileProgress(_T(""), false);
	UrlList.RemoveAll();
	ResultsWindow->Clear();
	EnablePrev(false);
	EnableNext();
	EnableExit(false);
	SetNextCaption(TR("Остановить"));

	int code = ResultsWindow->GetCodeType();
	int newcode = code;
	bool Thumbs = Settings.ThumbSettings.CreateThumbs!=0;

	// Корректировка типа кода в зависимости от включения превьюшек
	if(Thumbs)
	{
		if(code<4 && code>1)
			newcode = 0;
	}
	else
	{	
		if(code<2)
			newcode=2;
	}
	ResultsWindow->SetCodeType(newcode);
	ResultsWindow->SetPage(Settings.CodeLang);

	::SetFocus(GetDlgItem(IDC_CODEEDIT));
	Start();	
	return true;
}

bool CUploadDlg::OnNext()
{
	if(IsRunning())
	{
		if(!IsStopTimer)
		{
			SignalStop();
			CancelByUser = true;
			TimerInc = 5;
			SetTimer(1, 1000, NULL);
			IsStopTimer = true;
		}
		else 
		{
			if(TimerInc<5)
			{
				this->Terminate();
				ThreadTerminated();
				FileProgress(TR("Загрузка файлов прервана пользователем."), false);
			}
		}

	}
	else 
	{
		MainDlg->ThumbsView.MyDeleteAllItems();
		EnableExit();
		return true;
	}
	return false;
}

void CUploadDlg::ShowProgress(bool Show) {
	if(Show)
	{
		SetDlgItemText(IDC_SPEEDLABEL, _T(""));
		SetDlgItemText(IDC_PERCENTLABEL, _T("0%"));
		SetDlgItemText(IDC_FILEPROGRESS, _T(""));
	}

	if(Show) 
		SetTimer(2, 250); 
	
	if(!Show)
	{
		::ShowWindow(GetDlgItem(IDC_FILEPROGRESS),Show?SW_SHOW:SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_SPEEDLABEL),Show?SW_SHOW:SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_PERCENTLABEL),Show?SW_SHOW:SW_HIDE);
	}
}

bool CUploadDlg::OnHide() {
	UrlList.RemoveAll();
	ResultsWindow->Clear();
	Settings.UseTxtTemplate = (SendDlgItemMessage(IDC_USETEMPLATE, BM_GETCHECK) == BST_CHECKED);
	Settings.CodeType = ResultsWindow->GetCodeType();
	Settings.CodeLang = ResultsWindow->GetPage();
	return true; 
}

int GetWindowLeft(HWND Wnd)
{
	RECT WindowRect = {0,0,0,0};

	GetWindowRect(Wnd,&WindowRect);
	HWND Parent = GetParent(Wnd);
	ScreenToClient(Parent, (LPPOINT)&WindowRect);
	return WindowRect.left;
}

void CUploadDlg::FileProgress(const CString& Text, bool ShowPrefix)
{
	CString Temp;
	if(ShowPrefix)
	{ 
		Temp += TR("Текущий файл:"); 
		Temp += _T("  ");
	}

	Temp += Text;
	SetDlgItemText(IDC_INFOUPLOAD, Temp);

	bool IsProgressBar = ::IsWindowVisible(GetDlgItem(IDC_FILEPROGRESS))!=0;

	RECT rc;
	HWND Ctrl = GetDlgItem(IDC_INFOUPLOAD);
	::GetClientRect(Ctrl, &rc);
	int NewWidth = IsProgressBar?(GetWindowLeft(GetDlgItem(IDC_SPEEDLABEL))-GetWindowLeft(Ctrl)-5):400;
	if(NewWidth!=rc.right)
	{
		::SetWindowPos(Ctrl, 0, 0,0, NewWidth,rc.bottom,SWP_NOMOVE);
		::InvalidateRect(Ctrl, 0,0);
	}
}

void CUploadDlg::GenerateOutput() {
	ResultsWindow->UpdateOutput();
}

void CUploadDlg::SetUploadProgress(int CurPos, int Total, int FileProgress)
{
	SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETPOS, CurPos);
#if  WINVER	>= 0x0601 // Windows 7 related stuff
	if(ptl)
	{
		int NewCurrent = CurPos * 50 + FileProgress;
		int NewTotal = Total * 50;
		ptl->SetProgressValue(GetParent(), NewCurrent, NewTotal);
	}
#endif
	progressCurrent = CurPos;
	progressTotal = Total;
}

bool CUploadDlg::OnUploaderNeedStop()
{
	return m_bStopped;
}

void CUploadDlg::OnUploaderProgress(InfoProgress pi)
{
	PrInfo.CS.Lock();
	PrInfo.ip = pi;
	PrInfo.CS.Unlock();
}

void CUploadDlg::OnUploaderStatusChanged(StatusType status, int actionIndex, std::string text)
{
	m_StatusText = UploaderStatusToString(status, actionIndex,text);
}

bool CUploadDlg::OnConfigureNetworkManager(NetworkManager *nm) {
	IU_ConfigureProxy(*nm);
	return true;
}


const std::string Impl_AskUserCaptcha(NetworkManager *nm, const std::string& url)
{
	CString wFileName = WinUtils::GetUniqFileName(IUTempFolder+IuCoreUtils::Utf8ToWstring("captcha").c_str());

	nm->setOutputFile(IuCoreUtils::WstringToUtf8((const TCHAR*)wFileName));
	if(!nm->doGet(url))
		return "";
	CInputDialog dlg(_T("Image Uploader"), TR("Введите текст с картинки:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()),wFileName);
	nm->setOutputFile("");
	if(dlg.DoModal()==IDOK)
		return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
	return "";
}

bool CUploadDlg::OnFileFinished(bool ok,CFileQueueUploader:: FileListItem& result) {
	if ( !ok ) {
		//MessageBox( _T("Fail("));
		return false;
	} else {
		//MessageBoxA(0,result.fileName.c_str(),result.imageUrl.c_str(),0);
	}
	UploadTaskData *uploadTaskData =  reinterpret_cast<UploadTaskData*>( result.uploadTask->userData );
	if ( !uploadTaskData ) {
		return false;
	}
	UploadItem* uploadItem = uploadTaskData->uploadItem;

	if ( !uploadItem ) {
		return false;
	}

	if ( uploadTaskData->isThumb ) {
		uploadItem->thumbnailResult = result; 
		uploadItem->thumbnailResult.uploadTask = 0;
		uploadItem->thumbUploaded = true;
		uploadListView_.SetItemText(uploadItem->tableIndex, 2, _T("Миниатюра загружена"));
	} else {
		uploadItem->fileResult = result;
		uploadItem->fileUploaded = true;
		uploadItem->fileResult.uploadTask = 0;
	}
	
	ResultsWindow->Lock();
	mutex_->acquire();
	CUrlListItem item;
	int i = uploadItem->tableIndex;
	if ( uploadItem->needThumb && uploadItem->thumbUploaded && uploadItem->fileUploaded ) {
		item.ImageUrl = Utf8ToWCstring(uploadItem->fileResult.imageUrl);
		item.FileName = Utf8ToWCstring(uploadItem->fileResult.fileName);
		item.DownloadUrl = Utf8ToWCstring( uploadItem->fileResult.downloadUrl);

		item.ThumbUrl = Utf8ToWCstring( uploadItem->thumbnailResult.imageUrl);
		int columnIndex = uploadTaskData->isThumb ? 2 : 1;
		uploadListView_.SetItemText(i, 1, _T("Готово"));
		filesFinished_ ++;
		SetUploadProgress(filesFinished_,uploadItems_.size(),0);
		UrlList.Add(item);
	} else if ( uploadItem->fileUploaded && !uploadItem->needThumb  ) {
		item.ImageUrl = Utf8ToWCstring(uploadItem->fileResult.imageUrl);
		item.FileName = Utf8ToWCstring(uploadItem->fileResult.fileName);
		item.DownloadUrl = Utf8ToWCstring( uploadItem->fileResult.downloadUrl);
		item.ThumbUrl = Utf8ToWCstring( uploadItem->fileResult.thumbUrl);
		int columnIndex = uploadTaskData->isThumb ? 2 : 1;
		uploadListView_.SetItemText(i, 1, _T("Готово"));
		UrlList.Add(item);
		filesFinished_ ++;
		SetUploadProgress(filesFinished_,uploadItems_.size(),0);
	}

	CString res;
	res.Format(_T(" (%d)"), filesFinished_);
	 Toolbar.SetButtonInfo(IDC_UPLOADRESULTSTAB, TBIF_TEXT, 0, 0, CString(TR("Ссылки на файлы"))  +res , 
		0, 0,0, 0);
	//Toolbar.SetButtonInfo(TR("Ссылки на файлы"));

	
/*			CAbstractUploadEngine *upEngine = Uploader.getUploadEngine();*/
			std::string serverName =  "unknown";
			/*if(upEngine)
				serverName = upEngine->getUploadData()->Name;*/
			HistoryItem hi;
			hi.localFilePath = WCstringToUtf8(item.FileName);
			hi.serverName = serverName;
			hi.directUrl =  WCstringToUtf8(item.ImageUrl);
			hi.thumbUrl = WCstringToUtf8(item.ThumbUrl);
			hi.viewUrl = WCstringToUtf8(item.DownloadUrl);
			hi.uploadFileSize = result.fileSize;
			
			//session_.AddItem(hi);
			mutex_->release();
			
			ResultsWindow->Unlock();
			GenerateOutput();
addNewFilesToUploadQueue();
	return true;
}

bool CUploadDlg::OnQueueFinished(CFileQueueUploader*){
	//MessageBox(_T("OnQueueFinished"));
	tempFileDeleter_->Cleanup();
	SetUploadProgress(100,100,0);
	ThreadTerminated();
	showUploadResultsTab();
	return true;
}

bool CUploadDlg::OnUploadProgress(UploadProgress progress, UploadTask* task, NetworkManager* nm) {
	if ( !task)  {
		return false;
	}
	UploadTaskData *uploadTaskData =  reinterpret_cast<UploadTaskData*>(task->userData );
	if ( !uploadTaskData ) {
		return false;
	}
	UploadItem* uploadItem = uploadTaskData->uploadItem;
	if ( !uploadItem ) {
		return false;
	}
	int i = uploadItem->tableIndex;
	if ( i < 0 ) {
		return false;
	}
	
	int percent = 0;
	if ( progress.totalUpload ) {
		percent = 100 * ((float)progress.uploaded) / progress.totalUpload;
	}
	TCHAR ProgressBuffer[256]=_T("");
	_stprintf (ProgressBuffer,TR("Загружено %s из %s (%d%% )"),(LPCTSTR)Utf8ToWCstring(IuCoreUtils::fileSizeToString(progress.uploaded)),
		(LPCTSTR)Utf8ToWCstring(IuCoreUtils::fileSizeToString(progress.totalUpload)), percent);
	int columnIndex = uploadTaskData->isThumb ? 2 : 1;
	uploadListView_.SetItemText(i, columnIndex, ProgressBuffer);
	return true;
}
LRESULT CUploadDlg::OnLvnItemchangedUploadtable(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	return 0;
}

void CUploadDlg::createToolbar() {
	CBitmap hBitmap;
	HIMAGELIST m_hToolBarImageList;
	if (GuiTools::Is32BPP()) {
		hBitmap				  = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP3));
		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32,0,6);
		ImageList_Add(m_hToolBarImageList,hBitmap,NULL);
	} else {
		hBitmap = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDB_BITMAP4));
		m_hToolBarImageList = ImageList_Create(16,16,ILC_COLOR32 | ILC_MASK,0,6);
		ImageList_AddMasked(m_hToolBarImageList,hBitmap,RGB(255,0,255));
	}

	RECT rc = {0,0,100,24};
	GetClientRect(&rc);
	rc.top     = GuiTools::dlgY(24);
	rc.bottom  = rc.top + GuiTools::dlgY(16);
	rc.left    = GuiTools::dlgX(6);
	rc.right  -= GuiTools::dlgX(6);
	Toolbar.Create(m_hWnd,rc,_T(""), WS_CHILD|WS_CHILD | TBSTYLE_LIST |TBSTYLE_CUSTOMERASE|TBSTYLE_FLAT| CCS_NORESIZE/*|*/|CCS_BOTTOM | /*CCS_ADJUSTABLE|*/CCS_NODIVIDER|TBSTYLE_AUTOSIZE  );

	Toolbar.SetButtonStructSize();
	Toolbar.SetButtonSize(30,18);
	Toolbar.SetImageList(m_hToolBarImageList);
	Toolbar.AddButton(IDC_UPLOADPROCESSTAB, BTNS_CHECK|BTNS_AUTOSIZE ,TBSTATE_ENABLED|TBSTATE_PRESSED, 0, TR("Процесс загрузки"), 0);
	Toolbar.AddButton(IDC_UPLOADRESULTSTAB, BTNS_CHECK |BTNS_AUTOSIZE, TBSTATE_ENABLED, 1, TR("Ссылки на файлы"), 0);
	bool IsLastVideo = false;
	Toolbar.AddButton(IDC_MEDIAFILEINFO, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 2, TR("Инфо о последнем видео"), 0);
	Toolbar.AddButton(IDC_VIEWLOG, TBSTYLE_BUTTON |BTNS_AUTOSIZE, TBSTATE_ENABLED, 3, TR("Лог ошибок"), 0);


	if ( !IsLastVideo ) { 
		Toolbar.HideButton( IDC_MEDIAFILEINFO );
	}

	Toolbar.AutoSize();
	Toolbar.SetWindowLong(GWL_ID, IDC_RESULTSTOOLBAR);
	Toolbar.ShowWindow(SW_SHOW);
}

LRESULT CUploadDlg::OnUploadProcessButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	showUploadProgressTab();
	return 0;
}

LRESULT CUploadDlg::OnUploadResultsButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) {
	showUploadResultsTab();
	return 0;
}

LRESULT CUploadDlg::OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	if ( WizardDlg->LastVideoFile.IsEmpty() ) {
		return 0;
	}
	CMediaInfoDlg dlg(WizardDlg->LastVideoFile);
	dlg.DoModal();
	return 0;
}

LRESULT CUploadDlg::OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {	
	LogWindow.Show();
	return 0;
}

void CUploadDlg::showUploadResultsTab() {
	if ( currentTab_ == IDC_UPLOADRESULTSTAB ) {
		return;
	}
	Toolbar.SetButtonInfo(IDC_UPLOADPROCESSTAB, TBIF_STATE, 0, TBSTATE_ENABLED, 0 ,  0, 0,0, 0);
	Toolbar.SetButtonInfo(IDC_UPLOADRESULTSTAB, TBIF_STATE, 0, TBSTATE_ENABLED|TBSTATE_PRESSED, 0 ,  0, 0,0, 0);

	currentTab_ = IDC_UPLOADRESULTSTAB;
	uploadListView_.ShowWindow( SW_HIDE);
	ResultsWindow->ShowWindow( SW_SHOW );
}

void CUploadDlg::showUploadProgressTab() {
	if ( currentTab_ == IDC_UPLOADPROCESSTAB ) {
		return;
	}
	currentTab_= IDC_UPLOADPROCESSTAB;
	Toolbar.SetButtonInfo(IDC_UPLOADRESULTSTAB, TBIF_STATE, 0, TBSTATE_ENABLED, 0, 
		0, 0,0, 0);
	Toolbar.SetButtonInfo(IDC_UPLOADPROCESSTAB, TBIF_STATE, 0, TBSTATE_ENABLED|TBSTATE_PRESSED, 0 ,  0, 0,0, 0);

	uploadListView_.ShowWindow( SW_SHOW);
	ResultsWindow->ShowWindow( SW_HIDE );
}
void  CUploadDlg::EnableMediaInfo(bool Enable) {
	Toolbar.HideButton(IDC_MEDIAFILEINFO,!Enable);
}

bool CUploadDlg::getNextUploadItem() {
	if ( fileProcessingQueue_.empty() ) {
		return false;
	}
	FileProcessingStruct fps = fileProcessingQueue_.front();
	fileProcessingQueue_.pop_front();
	std::string fileName = fps.fileName;
	CUploadEngineData * uploadEngineData = fps.uploadEngineData;
	if ( uploadEngineData->SupportsFolders ) {
//		ResultsWindow->AddServer(Server);
	}
	std::string	sourceFileName = fileName;

/*	if(m_EngineList->byIndex(FileServer)->SupportsFolders) {
		//ResultsWindow->AddServer(Utf8ToWCstring(m_EngineList->byIndex(FileServer)->Name));
	}*/
	

//	//ShowProgress(false);
	//SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETPOS, 0);
//	SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, n));
	//SendDlgItemMessage(IDC_FILEPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	//SetDlgItemText(IDC_COMMONPERCENTS, _T("0%"));

	//	UploadProgress(0, n);

	CString FileName;
	TCHAR szBuffer[MAX_PATH];

	int NumUploaded=0;
	bool CreateThumbs = Settings.ThumbSettings.CreateThumbs;

	Thumbnail thumb;

	if(!thumb.LoadFromFile(WCstringToUtf8(IU_GetDataFolder()+_T("\\Thumbnails\\")+Settings.ThumbSettings.FileName+_T(".xml"))))
	{
		WriteLog(logError, _T("CThumbSettingsPage"), TR("Не могу загрузить файл миниатюры!"));
		return ThreadTerminated();
	}

	int thumbwidth = Settings.ThumbSettings.ThumbWidth;
	if ( thumbwidth < 1 || thumbwidth > 1024) {
		thumbwidth = 150;
	}

	FullUploadProfile iss;

	FullUploadProfile InitialParams;
	InitialParams.convert_profile = Settings.ConvertProfiles[Settings.CurrentConvertProfileName];
	InitialParams.upload_profile = Settings.UploadProfile;
	//= Settings.ImageSettings;
//	InitialParams.upload_profile.ServerID = Server;

	iss = InitialParams;

	UploadItem * newItem = new UploadItem;
		newItem->fileName   = fileName;
		newItem->tableIndex = fps.tableRow;
		newItem->needThumb  = false;
		uploadItems_.push_back(newItem);
		
		SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,0);
		CString ThumbFileName;

		CString ThumbUrl;
		CString DirectUrl;
		CString DownloadUrl;
//      Server = iss.upload_profile.ServerID;
		//ue = m_EngineList->byIndex(Server);
		CAbstractUploadEngine * e = 0;
		if(/*Server!=Uploader.CurrentServer && */IsImage(Utf8ToWCstring(fileName)))
		{
//			CUploadEngineData* data = m_EngineList->byIndex(Server);
			CUploadEngineData* newData = new CUploadEngineData();
			*newData =* uploadEngineData;
			e = new CDefaultUploadEngine();
			e->setUploadData(newData);
			ServerSettingsStruct& settings = Settings.ServerByUtf8Name(newData->Name);
			e->setServerSettings(settings);
			// e = m_EngineList->getUploadEngine(Server);
			if(!e) 
			{
				WriteLog(logError, _T("Custom Uploader"), _T("Cannot create image upload engine!"));
//				continue;
			}
//			Uploader.setUploadEngine(e);
		}

		SetDlgItemText(IDC_COMMONPROGRESS2,szBuffer);

		if(IsImage(Utf8ToWCstring(fileName)))
		{
			uploadListView_.SetItemText(fps.tableRow, 1, _T("Подготовка изображения"));
			//FileProgress(TR("Подготовка файла к отправке.."));
			if(ShouldStop()) 
				return ThreadTerminated();

			CImageConverter imageConverter;
         imageConverter.setImageConvertingParams(iss.convert_profile);
         imageConverter.setEnableProcessing(!iss.upload_profile.KeepAsIs);
			imageConverter.setThumbnail(&thumb);
			imageConverter.setThumbCreatingParams(Settings.ThumbSettings);
			bool GenThumb = false;
			if(CreateThumbs && ((!Settings.ThumbSettings.UseServerThumbs)||(!uploadEngineData->SupportThumbnails)))
				GenThumb = true;
				//GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,ThumbFileName,thumbwidth, iss);
			else 
				GenThumb = false;
				
			/*if(CreateThumbs && Settings.ThumbSettings.UseServerThumbs)
					GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,0,0, iss);

				else
					GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,0,0, iss);*/
			imageConverter.setGenerateThumb(GenThumb);
			imageConverter.Convert(Utf8ToWCstring(fileName));
			sourceFileName = WCstringToUtf8( imageConverter.getImageFileName() );
			ThumbFileName = imageConverter.getThumbFileName();
         if(!iss.upload_profile.KeepAsIs && sourceFileName != (fileName)) {
				//MessageBox(ImageFileName);
				tempFileDeleter_->AddFile(Utf8ToWCstring( sourceFileName ));
				if(lstrlen(ThumbFileName)) {
					tempFileDeleter_->AddFile(ThumbFileName);
				}
			}
		}
		else // if we upload any type of file
		{
			/*ImageFileName = Utf8ToWCstring(fileName);
			CAbstractUploadEngine * fileUploadEngine = m_EngineList->getUploadEngine(FileServer);
			if(!fileUploadEngine)
			{
				WriteLog(logError, _T("Custom Uploader"), _T("Cannot create file upload engine!"));
//				continue;
			}
			e = fileUploadEngine;	*/	
		}
	//	uploadListView_.SetItemText(fps.tableRow, 2, _T("Начинаем з"));
		FileName = Utf8ToWCstring(fileName);

		if ( !IuCoreUtils::FileExists ( sourceFileName ) ) {
			CString Buf;
			Buf.Format(TR("Файл \"%s\" не найден."), (LPCTSTR)IuCoreUtils::Utf8ToWstring(sourceFileName).c_str());
			WriteLog(logError, TR("Модуль загрузки"),Buf);
		//	continue;
		}

		if(IsImage(Utf8ToWCstring(fileName))&& uploadEngineData->MaxFileSize && IuCoreUtils::getFileSize(sourceFileName)>static_cast<int64_t>(uploadEngineData->MaxFileSize))
		{
			CSizeExceed SE(Utf8ToWCstring(sourceFileName), iss,m_EngineList);

			int res = SE.DoModal(m_hWnd);
			if(res==IDOK || res==3 )
			{
				if(res==3) InitialParams = iss; // if user choose button USE FOR ALL

//				i--;
				//continue;
			}
		}
		ShowProgress(true);

		/*if(IsImage(Utf8ToWCstring(fileName)))
			FileProgress(TR("Загрузка изображения.."));
		else 
			FileProgress(CString(TR("Загрузка файла")) + _T(" ") + myExtractFileName(Utf8ToWCstring(fileName)));*/

		DirectUrl= _T("");
		ThumbUrl = _T("");
		
//		CString virtualName = GetOnlyFileName(MainDlg->FileList[i].VirtualFileName)+_T(".")+GetFileExt(ImageFileName);
	//	Uploader.setThumbnailWidth(thumbwidth);
		
		queueUploader_.setUploadSettings(e);

		UploadTaskData * uploadTaskData = new UploadTaskData(newItem, false);
		uploadListView_.SetItemText(fps.tableRow, 1, _T("Ожидание загрузки"));
		queueUploader_.AddFile(sourceFileName, sourceFileName, uploadTaskData, e );

		queueUploader_.start();

		

		if(ShouldStop()) {
			return ThreadTerminated();
		}


					// Если мы не используем серверные превьюшки
			if(IsImage(Utf8ToWCstring(fileName)) &&  !uploadEngineData->ImageUrlTemplate.empty() 
				&& Settings.ThumbSettings.CreateThumbs 
				&& (((!Settings.ThumbSettings.UseServerThumbs)||(!uploadEngineData->SupportThumbnails)) 
				/*|| (lstrlen(ThumbUrl)<1)*/))
			{
//	thumb_retry:
			//	FileProgress(TR("Загрузка миниатюры.."));
			//	ShowProgress(true);
				ThumbUrl = _T("");
			/*	PrInfo.ip.Total=0;
				PrInfo.Bytes.clear();
				PrInfo.ip.Uploaded=0;
				PrInfo.ip.IsUploading = false;*/
				BOOL result = true
					/*Uploader.UploadFile(WCstringToUtf8(ThumbFileName),WCstringToUtf8(myExtractFileName(ThumbFileName)))*/;
				
				UploadTaskData * uploadTaskData = new UploadTaskData(newItem, true);
				newItem->needThumb = true;
				CDefaultUploadEngine * thumbEngine = new CDefaultUploadEngine();
				thumbEngine->setUploadData(uploadEngineData);
				ServerSettingsStruct& settings = Settings.ServerByUtf8Name(uploadEngineData->Name);
				thumbEngine->setServerSettings(settings);
			//	MessageBox(_T("Need !!!"));
				uploadListView_.SetItemText(fps.tableRow, 2, _T("Ожидание загрузки"));
				queueUploader_.AddFile(WCstringToUtf8(ThumbFileName), WCstringToUtf8(WinUtils::myExtractFileName(ThumbFileName)), uploadTaskData, thumbEngine);
				queueUploader_.start();
				/*if(result)
				{
					ThumbUrl = Utf8ToWstring(Uploader.getDirectUrl()).c_str();
				}*/
				if(ShouldStop()) 
					return ThreadTerminated();

				/*if(!result && Settings.ShowUploadErrorDialog)
				{
					CString Err;
					Err.Format(TR("Не удалось загрузить миниатюру к изображению \"%s\" на сервер. "), (LPCTSTR)MainDlg->FileList[i].FileName );
					int res = MessageBox(Err,APPNAME, MB_ABORTRETRYIGNORE|MB_ICONERROR);
					if(res == IDABORT) { ThreadTerminated();FileProgress(TR("Загрузка файлов прервана пользователем."), false);return 0;}
					else if(res == IDRETRY)
					{
						goto thumb_retry;
					}
				}*/
			}

		/*if(!result)
		{
			CString Err;
			Err.Format(TR("Не удалось загрузить файл \"%s\" на сервер. "), (LPCTSTR)MainDlg->FileList[i].FileName );
			if(Settings.ShowUploadErrorDialog)
			{
				int res = MessageBox(Err,APPNAME, MB_ABORTRETRYIGNORE|MB_ICONERROR);
				if(res == IDABORT) { ThreadTerminated();FileProgress(TR("Загрузка файлов прервана пользователем."), false);return 0;}
				else if(res == IDRETRY)
				{
					i--;
					continue;
				}
			}
		}*/



/*		if(result  &&  (!DirectUrl.IsEmpty() || !DownloadUrl.IsEmpty()))
		{
			if(uploadEngineData->SupportsFolders)
			{
//				ResultsWindow->AddServer(Utf8ToWCstring(ue->Name));
			}

			NumUploaded++;


			PrInfo.ip.Total=0;
			PrInfo.ip.Uploaded=0;
			LastUpdate=0;


			ShowProgress(false);
			CUrlListItem item;
			item.ImageUrl=_T("");
			item.ThumbUrl=_T("");

			item.ImageUrl = DirectUrl;
			item.FileName = Utf8ToWCstring(fileName);
			item.DownloadUrl = DownloadUrl;
			if(CreateThumbs)
				item.ThumbUrl = ThumbUrl;
			ResultsWindow->Lock();
			UrlList.Add(item);

/*			CAbstractUploadEngine *upEngine = Uploader.getUploadEngine();
			std::string serverName = ue->Name;
			if(upEngine)
				serverName = upEngine->getUploadData()->Name;
			HistoryItem hi;
			hi.localFilePath = WCstringToUtf8(item.FileName);
			hi.serverName = serverName;
			hi.directUrl =  WCstringToUtf8(DirectUrl);
			hi.thumbUrl = WCstringToUtf8(item.ThumbUrl);
			hi.viewUrl = WCstringToUtf8(item.DownloadUrl);
			hi.uploadFileSize = IuCoreUtils::getFileSize(WCstringToUtf8(ImageFileName));
			session.AddItem(hi);
			
			
			//ResultsWindow->Unlock();
		//	GenerateOutput();

		}*/
		ShowProgress(false);
//		UploadProgress(i+1, n);
//		wsprintf(szBuffer,_T("%d %%"),(int)((float)(i+1)/(float)n*100));
//		SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

//		ThumbUrl = _T("");
		DirectUrl = _T("");

		
		iss = InitialParams;
	
}

bool CUploadDlg::addNewFilesToUploadQueue() {
	CUploadEngineData * uploadEngineData = 0;
	int currentServerMaxThreads = 0;
	int addedFileCount = 0;
	do {
		if ( fileProcessingQueue_.empty() ) {
			break;
		}
		FileProcessingStruct nextProcessingItem = fileProcessingQueue_.front();
		uploadEngineData =  nextProcessingItem.uploadEngineData;
		currentServerMaxThreads = uploadEngineData->MaxThreadCount ? uploadEngineData->MaxThreadCount : 4;
		getNextUploadItem();
		addedFileCount ++;
	} while ( queueUploader_.isSlotAvailableForServer( uploadEngineData->Name, currentServerMaxThreads ) );
	return addedFileCount != 0;
}