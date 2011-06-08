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

#include "shobjidl.h"
#include "Core/ImageConverter.h"
#include "Func/Base.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Gui/Dialogs/InputDialog.h"
#include "Func/Settings.h"
#include "Core/Upload/UploadEngine.h"

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
}

CUploadDlg::~CUploadDlg()
{
	delete ResultsWindow;
}

Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);

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

	CVideoGrabber *vg =(	CVideoGrabber *) WizardDlg->Pages[1];

	if(vg && lstrlen(vg->m_szFileName))
		IsLastVideo=true;

	ResultsWindow->EnableMediaInfo(IsLastVideo);
	
	SetDlgItemInt(IDC_THUMBSPERLINE, 4);
	SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );
	
	MakeLabelBold(GetDlgItem(IDC_COMMONPROGRESS));
	MakeLabelBold(GetDlgItem(IDC_COMMONPERCENTS));
	PageWnd = m_hWnd;
	ResultsWindow->SetPage(Settings.CodeLang);
	
	ResultsWindow->SetCodeType(Settings.CodeType);

	return 1;  // Let the system set the focus
}

#define Terminat() {Terminated=true; return 0;}

DWORD CUploadDlg::Run()
{
	m_bStopped = false;
	HRESULT hRes = ::CoInitialize(NULL);
	CTempFilesDeleter  TempFilesDeleter;
	if(!MainDlg) return 0;

	CUploader  Uploader;
	
	#if  WINVER	>= 0x0601
		if(ptl)
			ptl->SetProgressState(GetParent(), TBPF_NORMAL); // initialise Windows 7 taskbar button progress 
	#endif
	m_CurrentUploader = &Uploader;
	Uploader.onNeedStop.bind(this, &CUploadDlg::OnUploaderNeedStop);
	Uploader.onProgress.bind(this, &CUploadDlg::OnUploaderProgress);
	Uploader.onDebugMessage.bind(DefaultErrorHandling::DebugMessage);
	Uploader.onErrorMessage.bind(DefaultErrorHandling::ErrorMessage);
	Uploader.onStatusChanged.bind(this, &CUploadDlg::OnUploaderStatusChanged);
	Uploader.onConfigureNetworkManager.bind(this, &CUploadDlg::OnUploaderConfigureNetworkClient);
	int Server;
	int FileServer = Settings.FileServerID;
	int n = MainDlg->FileList.GetCount();

	if(Settings.QuickUpload && !WizardDlg->Pages[3])
		Server = Settings.QuickServerID;
	else Server = Settings.ServerID;

	if(Server == -1)
	{
		Server = m_EngineList->getRandomImageServer();
		if(Server == -1)
			return ThreadTerminated();
	}

	if(FileServer == -1)
	{
		FileServer = m_EngineList->getRandomFileServer();
		if(FileServer == -1)
			return ThreadTerminated();
	}

	CUploadEngineData *ue = m_EngineList->byIndex(Server);
	if(ue->SupportsFolders)
	{
		ResultsWindow->AddServer(Server);
	}

	if(m_EngineList->byIndex(FileServer)->SupportsFolders)
	{
		ResultsWindow->AddServer(Utf8ToWCstring(m_EngineList->byIndex(FileServer)->Name));
	}

	ShowProgress(false);
	SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETPOS, 0);
	SendDlgItemMessage(IDC_UPLOADPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, n));
	SendDlgItemMessage(IDC_FILEPROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SetDlgItemText(IDC_COMMONPERCENTS, _T("0%"));

	UploadProgress(0, n);

	int i;
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

	int thumbwidth=Settings.ThumbSettings.ThumbWidth;
	if(thumbwidth<1|| thumbwidth>1024) thumbwidth=150;
	

	FullUploadProfile iss;


	FullUploadProfile InitialParams;
   InitialParams.convert_profile = Settings.ConvertProfiles[Settings.CurrentConvertProfileName];
	InitialParams.upload_profile = Settings.UploadProfile;
	//= Settings.ImageSettings;
   InitialParams.upload_profile.ServerID = Server;

	iss = InitialParams;

	CHistoryManager * mgr = ZBase::get()->historyManager();
	CHistorySession session = mgr->newSession();

	for(i=0; i<n; i++)
	{
		CString ImageFileName,ThumbFileName;

		CString ThumbUrl;
		CString DirectUrl;
		CString DownloadUrl;
      Server = iss.upload_profile.ServerID;
		ue = m_EngineList->byIndex(Server);
		if(/*Server!=Uploader.CurrentServer && */IsImage(MainDlg->FileList[i].FileName))
		{
			CAbstractUploadEngine * e = m_EngineList->getUploadEngine(Server);
			if(!e) 
			{
				WriteLog(logError, _T("Custom Uploader"), _T("Cannot create image upload engine!"));
				continue;
			}
			Uploader.setUploadEngine(e);
		}

		wsprintf(szBuffer,TR("Обрабатывается файл %d из %d..."),i+1,n/*,ExtractFileName(FileName),UrlBuffer*/);

		SetDlgItemText(IDC_COMMONPROGRESS2,szBuffer);

		SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,0);

		
		if(IsImage(MainDlg->FileList[i].FileName))
		{
			FileProgress(TR("Подготовка файла к отправке.."));
			if(ShouldStop()) 
				return ThreadTerminated();

			CImageConverter imageConverter;
         imageConverter.setImageConvertingParams(iss.convert_profile);
         imageConverter.setEnableProcessing(!iss.upload_profile.KeepAsIs);
			imageConverter.setThumbnail(&thumb);
			imageConverter.setThumbCreatingParams(Settings.ThumbSettings);
			bool GenThumb = false;
			if(CreateThumbs && ((!Settings.ThumbSettings.UseServerThumbs)||(!ue->SupportThumbnails)))
				GenThumb = true;
				//GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,ThumbFileName,thumbwidth, iss);
			else 
				GenThumb = false;
				
			/*if(CreateThumbs && Settings.ThumbSettings.UseServerThumbs)
					GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,0,0, iss);

				else
					GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,0,0, iss);*/
			imageConverter.setGenerateThumb(GenThumb);
			imageConverter.Convert(MainDlg->FileList[i].FileName);
			ImageFileName = imageConverter.getImageFileName();
			ThumbFileName = imageConverter.getThumbFileName();
         if(!iss.upload_profile.KeepAsIs && ImageFileName != MainDlg->FileList[i].FileName)
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
			CAbstractUploadEngine * fileUploadEngine = m_EngineList->getUploadEngine(FileServer);
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
		
		CString virtualName = GetOnlyFileName(MainDlg->FileList[i].VirtualFileName)+_T(".")+GetFileExt(ImageFileName);
		Uploader.setThumbnailWidth(thumbwidth);
		BOOL result = Uploader.UploadFile(WCstringToUtf8(ImageFileName),WCstringToUtf8(virtualName));
		if(result)
		{
			ThumbUrl = Utf8ToWstring(Uploader.getThumbUrl()).c_str();
			DirectUrl = Utf8ToWstring(Uploader.getDirectUrl()).c_str();
			DownloadUrl = Utf8ToWstring(Uploader.getDownloadUrl()).c_str();
		}
		
		ShowProgress(false);

		if(ShouldStop()) 
			return ThreadTerminated();

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

			// Если мы не используем серверные превьюшки
			if(IsImage(MainDlg->FileList[i].FileName) &&  !ue->ImageUrlTemplate.empty() && Settings.ThumbSettings.CreateThumbs && (((!Settings.ThumbSettings.UseServerThumbs)||(!ue->SupportThumbnails)) || (lstrlen(ThumbUrl)<1)))
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
			if(CreateThumbs)
				item.ThumbUrl = ThumbUrl;
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
			session.AddItem(hi);
			
			
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
		iss = InitialParams;
	}

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
	FileProgress(szBuffer, false);
	return ThreadTerminated();
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
			UploadProgress(progressCurrent, progressTotal, perc/2);

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
	bool IsLastVideo=false;

	if(lstrlen(MediaInfoDllPath))
	{
		CVideoGrabber *vg =(	CVideoGrabber *) WizardDlg->Pages[1];

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

void CUploadDlg::ShowProgress(bool Show)
{
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
	if(ShowPrefix){ 
		Temp+=TR("Текущий файл:"); Temp+=_T("  ");
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
	#if  WINVER	>= 0x0601
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

void CUploadDlg::OnUploaderConfigureNetworkClient(NetworkManager *nm)
{
	IU_ConfigureProxy(*nm);
}


const std::string Impl_AskUserCaptcha(NetworkManager *nm, const std::string& url)
{
	CString wFileName = GetUniqFileName(IUTempFolder+Utf8ToWstring("captcha").c_str());

	nm->setOutputFile(IuCoreUtils::WstringToUtf8((const TCHAR*)wFileName));
	if(!nm->doGet(url))
		return "";
	CInputDialog dlg(_T("Image Uploader"), TR("Введите текст с картинки:"), CString(IuCoreUtils::Utf8ToWstring("").c_str()),wFileName);
	nm->setOutputFile("");
	if(dlg.DoModal()==IDOK)
		return IuCoreUtils::WstringToUtf8((const TCHAR*)dlg.getValue());
	return "";
}