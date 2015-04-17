/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "UploadDlg.h"

#include <shobjidl.h>
#include "Core/Images/ImageConverter.h"
#include "Func/Base.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/InputDialog.h"
#include "Func/Settings.h"
#include "Core/Upload/UploadEngine.h"
#include "Gui/GuiTools.h"
#include <Func/LocalFileCache.h>
#include <Core/3rdpart/FastDelegate.h>
#include <Core/Upload/UrlShorteningTask.h>
#include <Core/Upload/FileQueueUploader.h>
#include <Func/IuCommonFunctions.h>
#include <Func/MyUtils.h>
#include <Core/Upload/FileUploadTask.h>
#include <Core/Upload/UploadManager.h>
#include <Func/WinUtils.h>

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
			result = CString(TR("Создание папки \"")) + Utf8ToWstring(param).c_str() + _T("\"...");
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
			result = Utf8ToWstring(param).c_str();
	}
	return result;
};

// Преобразование размера файла в строку
bool NewBytesToString(__int64 nBytes, LPTSTR szBuffer, int nBufSize)
{
	std::string res = IuCoreUtils::fileSizeToString(nBytes);
	lstrcpyn(szBuffer, Utf8ToWstring(res).c_str(), nBufSize);
	return TRUE;
}

bool BytesToString(__int64 nBytes, LPTSTR szBuffer,int nBufSize)
{
	return NewBytesToString(nBytes, szBuffer, nBufSize);
}

// CUploadDlg
CUploadDlg::CUploadDlg(CWizardDlg *dlg,UploadManager* uploadManager) :ResultsWindow(new CResultsWindow(dlg, UrlList, true))
{
	MainDlg = NULL;
	TimerInc = 0;
	IsStopTimer = false;
	Terminated = false;
	m_EngineList = _EngineList;
	LastUpdate = 0;
	//fastdelegate::FastDelegate1<bool> fd;
	//fd.bind(this, &CUploadDlg::onShortenUrlChanged);
	uploadManager_ = uploadManager;
	ResultsWindow->setOnShortenUrlChanged(fastdelegate::MakeDelegate(this, &CUploadDlg::onShortenUrlChanged));
	#if  WINVER	>= 0x0601
		ptl = NULL;
	#endif
}

CUploadDlg::~CUploadDlg()
{
	delete ResultsWindow;
}

//Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);

LRESULT CUploadDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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

	CVideoGrabberPage *vg = static_cast<CVideoGrabberPage*>(WizardDlg->Pages[1]);

	if(vg && lstrlen(vg->m_szFileName))
		IsLastVideo=true;

	ResultsWindow->EnableMediaInfo(IsLastVideo);

	SetDlgItemInt(IDC_THUMBSPERLINE, 4);
	SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
	
	GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPROGRESS));
	GuiTools::MakeLabelBold(GetDlgItem(IDC_COMMONPERCENTS));
	PageWnd = m_hWnd;
	ResultsWindow->SetPage(Settings.CodeLang);
	ResultsWindow->SetCodeType(Settings.CodeType);
	return 1;  
}

#define Terminat() {Terminated=true; return 0;}

DWORD CUploadDlg::Run()
{
	m_bStopped = false;
	HRESULT hRes = ::CoInitialize(NULL);
	CTempFilesDeleter  TempFilesDeleter;
	if(!MainDlg) return 0;
	
	#if  WINVER	>= 0x0601
		if(ptl)
			ptl->SetProgressState(GetParent(), TBPF_NORMAL); // initialise Windows 7 taskbar button progress 
	#endif
	/*Uploader.onNeedStop.bind(this, &CUploadDlg::OnUploaderNeedStop);
	Uploader.onProgress.bind(this, &CUploadDlg::OnUploaderProgress);
	Uploader.onDebugMessage.bind(DefaultErrorHandling::DebugMessage);
	Uploader.onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	Uploader.onStatusChanged.bind(this, &CUploadDlg::OnUploaderStatusChanged);
	Uploader.onConfigureNetworkClient.bind(this, &CUploadDlg::OnUploaderConfigureNetworkClient);*/

	/*if(Settings.QuickUpload && !WizardDlg->Pages[3]) {
		Server = Settings.getQuickServerID();
	}
	else Server = Settings.getServerID();*/
	/*ServerProfile& serverProfile = sessionImageServer_;
	int ServerId = _EngineList->GetUploadEngineIndex(sessionImageServer_.serverName());

	int FileServer = _EngineList->GetUploadEngineIndex(sessionFileServer_.serverName());
	

	if(ServerId == -1)
	{
		ServerId = m_EngineList->getRandomImageServer();
		if(ServerId == -1)
			return ThreadTerminated();
	}

	if(FileServer == -1)
	{
		FileServer = m_EngineList->getRandomFileServer();
		if(FileServer == -1)
			return ThreadTerminated();
	}

	CUploadEngineData *ue = m_EngineList->byIndex(ServerId);
	if (!ue  || sessionImageServer_.serverName().IsEmpty()) {
		WriteLog(logError, L"CUploadDlg", L"Server not selected");
		return ThreadTerminated();
	}
	if(ue->SupportsFolders)
	{
		ResultsWindow->AddServer(serverProfile);
	}

	if(m_EngineList->byIndex(FileServer)->SupportsFolders)
	{
		ResultsWindow->AddServer(Utf8ToWCstring(m_EngineList->byIndex(FileServer)->Name));
	}*/

	/*ShowProgress(false);
	SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETPOS, 0);
	SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, n));
	SendDlgItemMessage(IDC_FILEPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SetDlgItemText(IDC_COMMONPERCENTS, _T("0%"));

	UploadProgress(0, n);*/

	/*int i;
	CString FileName;
	TCHAR szBuffer[MAX_PATH];
	
	int NumUploaded=0;
	bool CreateThumbs = sessionImageServer_.getImageUploadParams().CreateThumbs;

	Thumbnail thumb;
	CString templateName = sessionImageServer_.getImageUploadParams().getThumb().TemplateName;
	if ( templateName.IsEmpty() ) {
		templateName = _T("default");
	}
	CString thumbTemplateFileName = IuCommonFunctions::GetDataFolder()+_T("\\Thumbnails\\")+templateName+_T(".xml");
	if(!thumb.LoadFromFile(WCstringToUtf8(thumbTemplateFileName)))
	{
		WriteLog(logError, _T("CUploadDlg"), TR("Не могу загрузить файл миниатюры!")+CString(_T("\r\n")) + thumbTemplateFileName);
		return ThreadTerminated();
	}

	int thumbwidth=sessionImageServer_.getImageUploadParams().getThumb().Size;
	if(thumbwidth<1|| thumbwidth>1024) thumbwidth=150;
	*/

	//= Settings.ImageSettings;
   //InitialParams.upload_profile.ServerID = Server;

	//iss = InitialParams;
	int n = MainDlg->FileList.GetCount();
	CHistoryManager * mgr = ZBase::get()->historyManager();
	std_tr::shared_ptr<CHistorySession> session = mgr->newSession();
	std::shared_ptr<UploadSession> uploadSession(new UploadSession());
	for(int i=0; i<n; i++)
	{
		CString FileName = MainDlg->FileList[i].FileName;
		std::string fileNameA = WCstringToUtf8(FileName);
		std::string displayName = WCstringToUtf8(MainDlg->FileList[i].VirtualFileName);

		bool isImage = IsImage(FileName);
		std::shared_ptr<FileUploadTask> task(new FileUploadTask(fileNameA, displayName));
		uploadSession->addTask(task);
		/*if ( isImage )
		{
			CAbstractUploadEngine * e = m_EngineList->getUploadEngine(ServerId, iss.upload_profile.serverSettings());
			if(!e) 
			{
				WriteLog(logError, _T("Custom Uploader"), _T("Cannot create image upload engine!"));
				continue;
			}
			Uploader.setUploadEngine(e);
		}
		std::shared_ptr<UploadTask> task(new FileUploadTask(WCstringToUtf8(FileName), ))

		wsprintf(szBuffer,TR("Обрабатывается файл %d из %d..."),i+1,n/*,ExtractFileName(FileName),UrlBuffer*);

		SetDlgItemText(IDC_COMMONPROGRESS2,szBuffer);

		SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,0);

		
		if(IsImage(MainDlg->FileList[i].FileName))
		{
			FileProgress(TR("Подготовка файла к отправке.."));
			if(ShouldStop()) 
				return ThreadTerminated();

			CImageConverter imageConverter;
         imageConverter.setImageConvertingParams(iss.convert_profile);
         imageConverter.setEnableProcessing(iss.upload_profile.getImageUploadParams().ProcessImages);
			imageConverter.setThumbnail(&thumb);
			imageConverter.setThumbCreatingParams(iss.upload_profile.getImageUploadParams().getThumb());
			bool GenThumb = false;
			if(CreateThumbs && ((!sessionImageServer_.getImageUploadParams().UseServerThumbs)||(!ue->SupportThumbnails)))
				GenThumb = true;
				//GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,ThumbFileName,thumbwidth, iss);
			else 
				GenThumb = false;

			imageConverter.setGenerateThumb(GenThumb);
			imageConverter.Convert(MainDlg->FileList[i].FileName);
			ImageFileName = imageConverter.getImageFileName();
			ThumbFileName = imageConverter.getThumbFileName();
         if(iss.upload_profile.getImageUploadParams().ProcessImages && ImageFileName != MainDlg->FileList[i].FileName)
			{
				//MessageBox(ImageFileName);
				TempFilesDeleter.AddFile(ImageFileName);
				if(lstrlen(ThumbFileName))
				TempFilesDeleter.AddFile(ThumbFileName);
			}
		}
		else // if we upload any type of file
		{
			ImageFileName = MainDlg->FileList[i].FileName;
			CAbstractUploadEngine * fileUploadEngine = m_EngineList->getUploadEngine(FileServer, sessionFileServer_.serverSettings());
			if(!fileUploadEngine)
			{
				WriteLog(logError, _T("Custom Uploader"), _T("Cannot create file upload engine!"));
				continue;
			}
			Uploader.setUploadEngine(fileUploadEngine);			
		}

		FileName = (MainDlg->FileList[i].FileName);
		SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,0);
		PrInfo.ip.Total=0;
		PrInfo.ip.Uploaded=0;
		PrInfo.Bytes.clear(); 
		PrInfo.ip.IsUploading = false;
		LastUpdate=0;
		ShowProgress(true);

		if(!FileExists(ImageFileName))
		{
			CString Buf;
			Buf.Format(TR("Файл \"%s\" не найден."), (LPCTSTR)ImageFileName);
			WriteLog(logError, TR("Модуль загрузки"),Buf);
			continue;
		}

		if(IsImage(MainDlg->FileList[i].FileName)&& ue->MaxFileSize && MyGetFileSize(ImageFileName)>static_cast<int>(ue->MaxFileSize))
		{
			CSizeExceed SE(ImageFileName, iss,m_EngineList);

			int res = SE.DoModal(m_hWnd);
			if(res==IDOK || res==3 )
			{
				if(res==3) InitialParams = iss; // if user choose button USE FOR ALL

				i--;
				continue;
			}
		}
		ShowProgress(true);

		if(IsImage(MainDlg->FileList[i].FileName))
			FileProgress(TR("Загрузка изображения.."));
		else 
			FileProgress(CString(TR("Загрузка файла")) + _T(" ") + myExtractFileName(MainDlg->FileList[i].FileName));

		DirectUrl= _T("");
		ThumbUrl = _T("");
		
		
		Uploader.setThumbnailWidth(thumbwidth);
		BOOL result = Uploader.UploadFile(WCstringToUtf8(ImageFileName),WCstringToUtf8(virtualName));
		if(result)
		{
			ThumbUrl = Utf8ToWstring(Uploader.getThumbUrl()).c_str();
			DirectUrl = Utf8ToWstring(Uploader.getDirectUrl()).c_str();
			DownloadUrl = Utf8ToWstring(Uploader.getDownloadUrl()).c_str();
		}
		
		ShowProgress(false);

		if(ShouldStop()) {
			return ThreadTerminated();
		}

		if(!result)
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
		}

		if(result  &&  (!DirectUrl.IsEmpty() || !DownloadUrl.IsEmpty()))
		{
			if(ue->SupportsFolders)
			{
				ResultsWindow->AddServer(Utf8ToWCstring(ue->Name));
			}

			NumUploaded++;


			PrInfo.ip.Total=0;
			PrInfo.ip.Uploaded=0;
			LastUpdate=0;
			//ShowVar((int)sessionImageServer_.getImageUploadParams().CreateThumbs);

			// Если мы не используем серверные превьюшки
			if(IsImage(MainDlg->FileList[i].FileName) &&  !ue->ImageUrlTemplate.empty() && sessionImageServer_.getImageUploadParams().CreateThumbs && (((!sessionImageServer_.getImageUploadParams().UseServerThumbs)||(!ue->SupportThumbnails)) || (lstrlen(ThumbUrl)<1)))
			{
	thumb_retry:
				FileProgress(TR("Загрузка миниатюры.."));
				ShowProgress(true);
				ThumbUrl = _T("");
				PrInfo.ip.Total=0;
				PrInfo.Bytes.clear();
				PrInfo.ip.Uploaded=0;
				PrInfo.ip.IsUploading = false;
				BOOL result = Uploader.UploadFile(WCstringToUtf8(ThumbFileName),WCstringToUtf8(myExtractFileName(ThumbFileName)));
				
				if(result)
				{
					ThumbUrl = Utf8ToWstring(Uploader.getDirectUrl()).c_str();
				}
				if(ShouldStop()) 
					return ThreadTerminated();

				if(!result && Settings.ShowUploadErrorDialog)
				{
					CString Err;
					Err.Format(TR("Не удалось загрузить миниатюру к изображению \"%s\" на сервер. "), (LPCTSTR)MainDlg->FileList[i].FileName );
					int res = MessageBox(Err,APPNAME, MB_ABORTRETRYIGNORE|MB_ICONERROR);
					if(res == IDABORT) { ThreadTerminated();FileProgress(TR("Загрузка файлов прервана пользователем."), false);return 0;}
					else if(res == IDRETRY)
					{
						goto thumb_retry;
					}
				}
			}
			ShowProgress(false);
			CUrlListItem item;
			item.ImageUrl=_T("");
			item.ThumbUrl=_T("");

			item.ImageUrl = DirectUrl;
			item.FileName = MainDlg->FileList[i].FileName;
			item.DownloadUrl = DownloadUrl;
			//if(CreateThumbs)
			if ( item.ThumbUrl.IsEmpty()) {
				item.ThumbUrl = ThumbUrl;
			}
			ResultsWindow->Lock();
			UrlList.Add(item);

			CAbstractUploadEngine *upEngine = Uploader.getUploadEngine();
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
			if ( !hi.directUrl.empty() ) {
				LocalFileCache::instance().addFile(hi.directUrl, hi.localFilePath);
			}
			session->AddItem(hi);
			
			
			ResultsWindow->Unlock();
			GenerateOutput();

		}
		ShowProgress(false);
		UploadProgress(i+1, n);
		wsprintf(szBuffer,_T("%d %%"),(int)((float)(i+1)/(float)n*100));
		SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

		ThumbUrl = _T("");
		DirectUrl = _T("");

		TempFilesDeleter.Cleanup();
		iss = InitialParams;*/
	}
	uploadManager_->addSession(uploadSession);
	/*

	UploadProgress(n, n);
	wsprintf(szBuffer,_T("%d %%"),100);
	SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

	
	int Errors = n-NumUploaded;
	if(Errors>0)
		wsprintf(szBuffer,CString(TR("Вcего %d файлов было загружено."))+_T(" ")
		+TR("%d файлов загружены не были из-за ошибок."),NumUploaded,Errors);
	else
		wsprintf(szBuffer,CString(TR("Вcего %d файлов было загружено."))+_T(" ")+TR("Ошибок нет."), NumUploaded );

	ResultsWindow->FinishUpload();
	FileProgress(szBuffer, false);*/
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

	else if(wParam == 2) // Another timer which updates status information in the window
	{

		/*PrInfo.CS.Lock();
		if(!PrInfo.ip.IsUploading) 
		{
			ShowProgress(false);
			PrInfo.ip.clear();
			PrInfo.Bytes.clear();
				
			if(m_CurrentUploader)
			{

				//ShowVar(m_CurrentUploader->GetStatus());
				if ( m_CurrentUploader->GetStatus() == stUploading ) {
					m_CurrentUploader->SetStatus(stWaitingAnswer);
				}
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
				if(!::IsWindowVisible(GetDlgItem(IDC_FILEPROGRESS)) )
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
			UploadProgress(progressCurrent, progressTotal, perc/2);

			DWORD Current =  PrInfo.ip.Uploaded;
			TCHAR SpeedBuffer[256]=_T("\0");
			buf1[0]=0;


			int speed=0;
			if(PrInfo.Bytes.size())
			{
				speed= int(((double)(Current-PrInfo.Bytes[0])/(double)PrInfo.Bytes.size())*4);
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
		PrInfo.CS.Unlock();*/
	}
	return 0;
}

int CUploadDlg::ThreadTerminated(void)
{
	WizardDlg->QuickUploadMarker = false;
	TCHAR szBuffer[MAX_PATH];
	SetDlgItemText(IDC_COMMONPROGRESS2, TR("Загрузка завершена."));
	UploadProgress(MainDlg->FileList.GetCount(), MainDlg->FileList.GetCount());
	#if  WINVER	>= 0x0601
		if(ptl)
			ptl->SetProgressState(GetParent(), TBPF_NOPROGRESS);
	#endif
	wsprintf(szBuffer,_T("%d %%"),100);
	SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

	if(CancelByUser)
		FileProgress(TR("Загрузка файлов прервана пользователем."), false);
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
	sessionFileServer_ = WizardDlg->getSessionFileServer();
	sessionImageServer_ = WizardDlg->getSessionImageServer();
	bool IsLastVideo=false;

	if(lstrlen(MediaInfoDllPath))
	{
		CVideoGrabberPage *vg =(	CVideoGrabberPage *) WizardDlg->Pages[1];

		if(vg && lstrlen(vg->m_szFileName))
			IsLastVideo=true;
	}
	ResultsWindow->InitUpload();
	ResultsWindow->EnableMediaInfo(IsLastVideo);
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
	bool Thumbs = sessionImageServer_.getImageUploadParams().CreateThumbs!=0;

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
	alreadyShortened_ = false;
	Run();	
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

void CUploadDlg::ShowProgress(bool Show)
{
	if(Show)
	{
		SetDlgItemText(IDC_SPEEDLABEL, _T(""));
		SetDlgItemText(IDC_PERCENTLABEL, _T("0%"));
		SetDlgItemText(IDC_FILEPROGRESS, _T(""));
		SetTimer(2, 250); 
	}	
	
	if(!Show)
	{
		::ShowWindow(GetDlgItem(IDC_FILEPROGRESS),Show?SW_SHOW:SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_SPEEDLABEL),Show?SW_SHOW:SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_PERCENTLABEL),Show?SW_SHOW:SW_HIDE);
	}
}

bool CUploadDlg::OnHide()
{
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

void CUploadDlg::GenerateOutput()
{
	ResultsWindow->UpdateOutput();
}

void CUploadDlg::UploadProgress(int CurPos, int Total, int FileProgress)
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

void CUploadDlg::OnUploaderProgress(CUploader* uploader, InfoProgress pi)
{
	PrInfo.CS.Lock();
	PrInfo.ip = pi;
	PrInfo.CS.Unlock();
}

void CUploadDlg::OnUploaderStatusChanged(StatusType status, int actionIndex, std::string text)
{
	PrInfo.CS.Lock();
	m_StatusText = UploaderStatusToString(status, actionIndex,text);
	PrInfo.Bytes.clear(); 
	PrInfo.ip.clear();
	PrInfo.CS.Unlock();
}

void CUploadDlg::OnUploaderConfigureNetworkClient(NetworkClient *nm)
{
	IU_ConfigureProxy(*nm);
}




void CUploadDlg::onShortenUrlChanged(bool shortenUrl) {
	if ( !alreadyShortened_ && shortenUrl ) {
		int len = UrlList.GetCount();
		for ( int i = 0; i < len; i++ ) {
			AddShortenUrlTask(&UrlList[i]);
		}
		alreadyShortened_ = true;
	} else {
		GenerateOutput();
	}
}

void CUploadDlg::AddShortenUrlTask(CUrlListItem* item) {
	/*if ( !item->ImageUrl.IsEmpty() && item->ImageUrlShortened.IsEmpty() ) {
		AddShortenUrlTask(item, _T("ImageUrl") );
	}
	if ( !item->DownloadUrl.IsEmpty() && item->DownloadUrlShortened.IsEmpty() ) {
		AddShortenUrlTask(item, _T("DownloadUrl") );
	}*/
	/*if ( !item->ThumbUrl.IsEmpty() && item->ThumbUrlShortened.IsEmpty() ) {
		AddShortenUrlTask(item, _T("ThumbUrl") );
	}*/
}

void CUploadDlg::AddShortenUrlTask(CUrlListItem* item, CString linkType) {
	/*CUploadEngineData *ue = Settings.urlShorteningServer.uploadEngineData();
	if ( !ue ) {
		WriteLog(logError, _T("Uploader"), _T("Cannot create url shortening engine '" + Settings.urlShorteningServer.serverName() + "'"));
		return;
	}
	CUploadEngineData* newData = new CUploadEngineData();
	*newData = *ue;
	CAbstractUploadEngine * e = m_EngineList->getUploadEngine(ue,Settings.urlShorteningServer.serverSettings());
	e->setUploadData(newData);
	ServerSettingsStruct& settings = Settings.urlShorteningServer.serverSettings();
	e->setServerSettings(settings);
	ShortenUrlUserData* userData = new ShortenUrlUserData;
	userData->item = item;
	userData->linkType = linkType;
	CString url;
	if ( linkType == _T("ImageUrl") ) {
		url = item->ImageUrl;
	} else if ( linkType == _T("DownloadUrl") ) {
		url = item->DownloadUrl;
	} else if ( linkType == _T("ThumbUrl") ) {
		url = item->ThumbUrl;
	}
	
	if ( url.IsEmpty() ) {
		return;
	}
	std_tr::shared_ptr<UrlShorteningTask> task(new UrlShorteningTask(WCstringToUtf8(url)));
	queueUploader_->AddUploadTask(task, reinterpret_cast<void*>(userData), e);
	queueUploader_->start();*/
}

bool CUploadDlg::OnFileFinished(std::shared_ptr<UploadTask> task, bool ok) {
	/*ShortenUrlUserData* shortenUrlUserData  = reinterpret_cast<ShortenUrlUserData*>(result.uploadTask->userData);
		
	if ( shortenUrlUserData->linkType == "ImageUrl" ){
		shortenUrlUserData->item->ImageUrlShortened = Utf8ToWCstring(result.imageUrl);
	}

	if ( shortenUrlUserData->linkType == "DownloadUrl" ) {
		shortenUrlUserData->item->DownloadUrlShortened = Utf8ToWCstring(result.imageUrl);
	}

	if ( shortenUrlUserData->linkType == "ThumbUrl" ) {
		shortenUrlUserData->item->ThumbUrlShortened = Utf8ToWCstring(result.imageUrl);
	}*/

	GenerateOutput();

	return false;
}

bool CUploadDlg::OnConfigureNetworkClient(CFileQueueUploader*, NetworkClient* nm) {
	IU_ConfigureProxy(*nm);
	return true;
}