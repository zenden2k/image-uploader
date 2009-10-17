/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2009 ZendeN <zenden2k@gmail.com>
	 
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

#include "stdafx.h"

#include "UploadDlg.h"
#include "shobjidl.h"


// Преобразование размера файла в строку


bool NewBytesToString(__int64 nBytes, LPTSTR szBuffer, int nBufSize)
{
	//TCHAR szMeasureNames[4][5]={"Byte","МB","KB","GB"};
	LPTSTR szName;
	szName=_T("Bytes");
	double number=0;
	int id=0;

	if(nBytes<1024)
	{
		wsprintf(szBuffer,_T("%d %s"),(int)nBytes, szName);
	}
	else{

		if(nBytes>=1024 && nBytes<(1048576))
		{
			number= (double)nBytes / 1024.0;
			szName=_T("kB");

		}
		else if(nBytes>=1048576 && (nBytes<(__int64(1073741824) /*1 GB*/)))
		{
			szName=_T("mB");
			number= (double)nBytes / 1048576.0;
		}

		else if(nBytes>=1073741824)
		{
			szName=_T("gB");
			number= (double)nBytes / 1073741824.0;
		}
		swprintf(szBuffer,_T("%1.0f %s"),number,szName);
	}
	return TRUE;
}

bool BytesToString(__int64 nBytes, LPTSTR szBuffer,int nBufSize)
{
	LPTSTR szName;
	szName = _T("Bytes");
	double number=0;
	int id=0;
	if(nBytes<1024)
	{
		wsprintf(szBuffer,_T("%d %s"),(int)nBytes,szName);
	}
	else{

		if(nBytes>=1024 && nBytes<(1048576))
		{
			number= (double)nBytes / 1024.0;
			szName=_T("KB");
			wsprintf(szBuffer,_T("%d %s"),(int)number,szName);
			return TRUE;

		}
		else if(nBytes>=1048576 && (nBytes<(__int64(1073741824) /*1 GB*/)))
		{
			szName=_T("MB");
			number= (double)nBytes / 1048576.0;
		}

		else
		{
			szName = _T("GB");
			number = (double)nBytes / 1073741824.0;
		}
		swprintf(szBuffer,_T("%3.2f %s"),number,szName);
	}
	return TRUE;
}

// CUploadDlg
CUploadDlg::CUploadDlg(CWizardDlg *dlg):ResultsPanel(UrlList,dlg)
{
	MainDlg = NULL;
	TimerInc = 0;
	IsStopTimer = false;
	Terminated = false;

	LastUpdate = 0;
	#if  WINVER	>= 0x0700
		ptl = NULL;
	#endif
}

CUploadDlg::~CUploadDlg()
{
	
}

Bitmap* BitmapFromResource(HINSTANCE hInstance,LPCTSTR szResName, LPCTSTR szResType);

LRESULT CUploadDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#if  WINVER	>= 0x0700
	const GUID IID_ITaskbarList3 = { 0xea1afb91,0x9e28,0x4b86,{0x90,0xe9,0x9e,0x9f, 0x8a,0x5e,0xef,0xaf}};
	CoCreateInstance(
		CLSID_TaskbarList, NULL, CLSCTX_ALL,
		IID_ITaskbarList3, (void**)&ptl);
#endif

	TC_ITEM item;
	item.pszText = TR("Для форума"); 
	item.mask = TCIF_TEXT	;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 0, &item);
	item.pszText = TR("Для сайта"); 
	item.mask = TCIF_TEXT	;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 1, &item);
	item.pszText = TR("Просто ссылки"); 
	item.mask = TCIF_TEXT	;
	TabCtrl_InsertItem(GetDlgItem(IDC_RESULTSTAB), 2, &item);
	TabBackgroundFix(GetDlgItem(IDC_CODETYPELABEL));
	///----
	TRC(IDC_COMMONPROGRESS, "Общий прогресс:");

	bool IsLastVideo=lstrlen(MediaInfoDllPath);

	CVideoGrabber *vg =(	CVideoGrabber *) WizardDlg->Pages[1];

	if(vg && lstrlen(vg->m_szFileName))
		IsLastVideo=true;

	ResultsPanel.EnableMediaInfo(IsLastVideo);

	// Creating panel with results
	RECT rc = {150,3,636,300};
	ResultsPanel.Create(m_hWnd,rc);
	RECT Rec;
	ResultsPanel.GetClientRect(&rc);
	TabCtrl_AdjustRect(GetDlgItem(IDC_RESULTSTAB),FALSE, &rc); 	
	::GetWindowRect(GetDlgItem(IDC_RESULTSTAB),&Rec);
	POINT p={Rec.left, Rec.top};
	ScreenToClient(&p);

	rc.left+=p.x;
	rc.top+=p.y;
	rc.right+=p.x;
	rc.bottom+=p.y;

	ResultsPanel.SetWindowPos(0,rc.left,rc.top,rc.right,rc.bottom,SWP_NOSIZE);
	SetDlgItemInt(IDC_THUMBSPERLINE, 4);
	SendDlgItemMessage(IDC_THUMBPERLINESPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG((short)100, (short)1) );

	MakeLabelBold(GetDlgItem(IDC_COMMONPROGRESS));
	MakeLabelBold(GetDlgItem(IDC_COMMONPERCENTS));
	int codetype = Settings.CodeType;
	//ReadSetting(_T("Upload.Codetype"),&codetype,0);
	if(codetype<0|| codetype>4) codetype=0;
	SendDlgItemMessage(IDC_CODETYPE,CB_SETCURSEL,codetype);	
	PageWnd = m_hWnd;
	ResultsPanel.SetPage(Settings.CodeLang);
	TabCtrl_SetCurSel(GetDlgItem(IDC_RESULTSTAB), Settings.CodeLang);
	ResultsPanel.SetCodeType(codetype);

	return 1;  // Let the system set the focus
}

#define Terminat() {Terminated=true; return 0;}

DWORD CUploadDlg::Run()
{
	HRESULT hRes = ::CoInitialize(NULL);

	if(!MainDlg) return 0;

	CUploader Uploader;

#if  WINVER	>= 0x0700
	if(ptl)
		ptl->SetProgressState(GetParent(), TBPF_NORMAL); // initialise Windows 7 taskbar button progress 
#endif
	m_CurrentUploader = &Uploader;

	Uploader.ShouldStop=&m_bStopped;
	Uploader.ProgressBuffer = ProgressBuffer;
	Uploader.PrInfo=&PrInfo;

	int Server;
	
	int n = MainDlg->FileList.GetCount();


	if(Settings.QuickUpload && !WizardDlg->Pages[3])
		Server = Settings.QuickServerID;
	else Server = Settings.ServerID;

	if(!Uploader.SelectServer(Server))
	{
		MessageBox(_T("?"));
		ThreadTerminated();
		EndDialog(0);
		return 0;
	}

	SendDlgItemMessage(IDC_UPLOADPROGRESS,PBM_SETPOS,0);
	ShowProgress(false);
	SendDlgItemMessage(IDC_UPLOADPROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, n));
	SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, 100));
	SetDlgItemText(IDC_COMMONPERCENTS,_T("0%"));

	UploadProgress(0, n);

	int i;
	LPTSTR FileName;
	TCHAR szBuffer[MAX_PATH];
	TCHAR UrlBuffer[256]=_T("\0");
	int NumUploaded=0;
	bool CreateThumbs=Settings.ThumbSettings.CreateThumbs;

	int thumbwidth=Settings.ThumbSettings.ThumbWidth;
	if(thumbwidth<1|| thumbwidth>1024) thumbwidth=150;
	TCHAR ImageFileName[256]=_T("\0"),ThumbFileName[256]=_T("\0");
	ImageSettingsStruct iss;

	ImageSettingsStruct InitialParams = Settings.ImageSettings;
	InitialParams.ServerID = Server;

	iss = InitialParams;


	for(i=0; i<n; i++)
	{
		Server = iss.ServerID;

		if(Server!=Uploader.CurrentServer && IsImage(MainDlg->FileList[i].FileName))
		{

			Uploader.SelectServer(Server);
		}


		*UrlBuffer = 0;
		wsprintf(szBuffer,TR("Обрабатывается файл %d из %d..."),i+1,n/*,ExtractFileName(FileName),UrlBuffer*/);

		SetDlgItemText(IDC_COMMONPROGRESS2,szBuffer);

		SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,0);

		if(IsImage(MainDlg->FileList[i].FileName))
		{


			FileProgress(TR("Подготовка файла к отправке.."));
			// SetDlgItemText(IDC_INFOUPLOAD,TR("Подготовка файла к отправке.."));

			if(ShouldStop()) 
				return ThreadTerminated();

			*ThumbFileName=0;
			*ImageFileName=0;

			if(CreateThumbs && ((!Settings.ThumbSettings.UseServerThumbs)||(!Uploader.ServerSupportThumbnails(Server)))/*&&MainDlg->FileList[i].ImageParams.GenThumb*/)

				GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,ThumbFileName,thumbwidth, iss);
			else 
				if(CreateThumbs && Settings.ThumbSettings.UseServerThumbs/*&&MainDlg->FileList[i].ImageParams.GenThumb*/)
					GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,0,0, iss);

				else
					GenerateImages(MainDlg->FileList[i].FileName,ImageFileName,0,0, iss);

		}
		else // if we upload any type of file
		{
			lstrcpy(ImageFileName, MainDlg->FileList[i].FileName);
			Uploader.SelectServer(Settings.FileServerID);
		}

		FileName = (MainDlg->FileList[i].FileName);


		TCHAR ThumbUrl[256];
		*ThumbUrl=0;
		SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,0);
		PrInfo.Total=0;
		PrInfo.Uploaded=0;
		PrInfo.IsUploading = false;
		LastUpdate=0;
		ShowProgress(true);

		if(!FileExists(ImageFileName))
		{
			TCHAR Buf[256];
			wsprintf(Buf, TR("Файл \"%s\" не найден."),ImageFileName);
			WriteLog(logError, TR("Модуль загрузки"),Buf);
			continue;
		}

		if(IsImage(MainDlg->FileList[i].FileName)&& EnginesList[Server].MaxFileSize && MyGetFileSize(ImageFileName)>EnginesList[Server].MaxFileSize)
		{
			CSizeExceed SE(ImageFileName, iss);

			int res = SE.DoModal(m_hWnd);
			if(res==IDOK || res==3 )
			{
				if(res==3) InitialParams = iss;

				i--;
				continue;
			}
		}
		ShowProgress(true);
		if(IsImage(MainDlg->FileList[i].FileName))
			FileProgress(TR("Загрузка картинки.."));
		else 
			FileProgress(CString(TR("Загрузка файла"))+_T(" ")+myExtractFileName(MainDlg->FileList[i].FileName));

		*UrlBuffer=0;
		*ThumbUrl = 0;
		CString DownloadUrl;
		BOOL result = Uploader.UploadFile(ImageFileName,UrlBuffer,ThumbUrl,thumbwidth);
		DownloadUrl = Uploader.getDownloadUrl();

		ShowProgress(false);
		if(ShouldStop()) 
			return ThreadTerminated();

		if(!result)
		{
			CString Err;
			Err.Format(TR("Не удалось загрузить файл \"%s\" на сервер. "), MainDlg->FileList[i].FileName );
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

		if(result  &&  (lstrlen(UrlBuffer)>0 || !DownloadUrl.IsEmpty()))
		{
			NumUploaded++;


			PrInfo.Total=0;
			PrInfo.Uploaded=0;
			LastUpdate=0;

			// Если мы не используем серверные превьюшки
			if(IsImage(MainDlg->FileList[i].FileName) &&  !EnginesList[Server].ImageUrlTemplate.IsEmpty() && Settings.ThumbSettings.CreateThumbs && (((!Settings.ThumbSettings.UseServerThumbs)||(!Uploader.ServerSupportThumbnails(Server))) || (lstrlen(ThumbUrl)<1)))
			{
	thumb_retry:
				FileProgress(TR("Загрузка миниатюры.."));
				ShowProgress(true);
				*ThumbUrl = 0;
				PrInfo.Total=0;
				PrInfo.Uploaded=0;
				PrInfo.IsUploading = false;
				BOOL result = Uploader.UploadFile(ThumbFileName,ThumbUrl,0);
				if(ShouldStop()) 
					return ThreadTerminated();

				if(!result && Settings.ShowUploadErrorDialog)
				{
					CString Err;
					Err.Format(TR("Не удалось загрузить миниатюру к изображению \"%s\" на сервер. "), MainDlg->FileList[i].FileName );
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
			*item.ImageUrl=0;
			*item.ThumbUrl=0;

			lstrcpy(item.ImageUrl,UrlBuffer);
			item.FileName = MainDlg->FileList[i].FileName;
			item.DownloadUrl= DownloadUrl;
			if(CreateThumbs)
				lstrcpy(item.ThumbUrl,ThumbUrl);
			ResultsPanel.UrlListCS.Lock();
			UrlList.Add(item);
			ResultsPanel.UrlListCS.Unlock();
			GenerateOutput();

		}
		ShowProgress(false);
		UploadProgress(i+1, n);
		wsprintf(szBuffer,_T("%d %%"),(int)((float)(i+1)/(float)n*100));
		SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

		*ThumbUrl=0;
		*UrlBuffer=0;

		iss = InitialParams;
	}

	UploadProgress(n, n);
	//SendDlgItemMessage(IDC_UPLOADPROGRESS,PBM_SETPOS,n, 0);
	wsprintf(szBuffer,_T("%d %%"),100);
	SetDlgItemText(IDC_COMMONPERCENTS,szBuffer);

	SetDlgItemText(IDC_COMMONPROGRESS2,TR("Загрузка была завершена."));
	int Errors = n-NumUploaded;
	if(Errors>0)
		wsprintf(szBuffer,CString(TR("Вcего %d файлов было загружено."))+_T(" ")
		+TR("%d файлов загружены не были из-за ошибок."),NumUploaded,Errors);
	else
		wsprintf(szBuffer,CString(TR("Вcего %d файлов было загружено."))+_T(" ")+TR("Ошибок нет."), NumUploaded, Errors);

	BOOL temp;
	ResultsPanel.OnCbnSelchangeCodetype(0,0,0,temp);
	FileProgress(szBuffer, false);

	return ThreadTerminated();
}

LRESULT CUploadDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam==1) // This timer is enabled when user pressed "Stop" button
	{
		TimerInc--;
		TCHAR szBuffer[256];
		if(TimerInc>0)
		{
			wsprintf(szBuffer,CString(TR("Остановить"))+_T(" (%d)"),TimerInc);
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

	else if(wParam==2) // The timer which updates status information int the window
	{

		PrInfo.CS.Lock();
		if(!PrInfo.IsUploading) 
		{
			if(m_CurrentUploader)
			{
				CString StatusText = m_CurrentUploader->GetStatusText();
				if(!StatusText.IsEmpty())
					FileProgress(StatusText);
			}
			PrInfo.CS.Unlock();
			return 0;

		}
		else
			if(PrInfo.Total>0)
		 {

			 TCHAR buf1[100],buf2[100];
			 BytesToString(PrInfo.Uploaded,buf1,sizeof(buf1));
			 BytesToString(PrInfo.Total,buf2,sizeof(buf2));

			 int perc = ((float)PrInfo.Uploaded/(float)PrInfo.Total)*100;
			 if(perc<0 || perc>100) perc=0;
			 if(perc>=100) {
				 ShowProgress(false);
				 PrInfo.Uploaded=0;
				 PrInfo.Total = 0;
				 PrInfo.IsUploading  = false;
				 PrInfo.CS.Unlock();
				 return 0;
			 }
			 if(PrInfo.IsUploading) 
			 {
				 if(!::IsWindowVisible(GetDlgItem(IDC_FILEPROGRESS)) /*&& PrInfo.Total==0*/)
				 {
					::ShowWindow(GetDlgItem(IDC_FILEPROGRESS),SW_SHOW);
					 ::ShowWindow(GetDlgItem(IDC_SPEEDLABEL),SW_SHOW);
					 ::ShowWindow(GetDlgItem(IDC_PERCENTLABEL),SW_SHOW);
				 }
			 }
			 _stprintf (ProgressBuffer,TR("Загружено %s из %s"),buf1,buf2,(int)perc);
			 FileProgress(ProgressBuffer);
			 wsprintf (ProgressBuffer,_T ("%d %%"),(int)perc);
			 SetDlgItemText(IDC_PERCENTLABEL,ProgressBuffer);
			 SendDlgItemMessage(IDC_FILEPROGRESS,PBM_SETPOS,(int)perc);
			 UploadProgress(progressCurrent, progressTotal, perc/2);

			 DWORD Current =  PrInfo.Uploaded;
			 TCHAR SpeedBuffer[256]=_T("\0");
			 buf1[0]=0;
			


				 int speed=0;
				 if(PrInfo.Bytes.size()){
					 speed= ((double)(Current-/*LastUpdate*/PrInfo.Bytes[0])/(double)PrInfo.Bytes.size())*4;}
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
	WizardDlg->QuickUploadMarker=false;
	TCHAR szBuffer[MAX_PATH];
	SetDlgItemText(IDC_COMMONPROGRESS2,TR("Загрузка была завершена."));
	UploadProgress(MainDlg->FileList.GetCount(), MainDlg->FileList.GetCount());
#if  WINVER	>= 0x0700
	if(ptl)
		ptl->SetProgressState(GetParent(), TBPF_NOPROGRESS); // initialise Windows 7 taskbar button progress 
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
	Terminated=true;
	IsStopTimer = false;
	EnablePrev();
	EnableNext();
	EnableExit();
	return 0;
}


void DrawGradient(Graphics &gr,Rect rect,Color &Color1,Color &Color2)
{
	Bitmap bm(rect.Width,rect.Height,&gr);
	Graphics temp(&bm);
	LinearGradientBrush 
		brush(/*TextBounds*/Rect(0,0,rect.Width,rect.Height), Color1, Color2,LinearGradientModeVertical);

	temp.FillRectangle(&brush,Rect(0,0,rect.Width,rect.Height));
	gr.DrawImage(&bm, rect.X,rect.Y);
}


void DrawRect(Bitmap &gr,Color &color,Rect rect)
{
	int i;
	SolidBrush br(color);
	for(i=rect.X;i<rect.Width;i++)
	{
		gr.SetPixel(i,0,color);
		gr.SetPixel(i,rect.Height-1,color);
	}

	for(i=rect.Y;i<rect.Height;i++)
	{
		gr.SetPixel(0,i,color);
		gr.SetPixel(rect.Width-1,i,color);
	}
}

#define MYRGB(a,color) Color(a,GetRValue(color),GetGValue(color),GetBValue(color))

int CUploadDlg::GenerateImages(LPTSTR szFileName, LPTSTR szBufferImage, LPTSTR szBufferThumb,int thumbwidth, ImageSettingsStruct &iss)
{
	RECT rc;
	GetClientRect(&rc);

	//	UPLOADPARAMS* item = &WizardDlg->UploadParams;
	//LogoParams params = logoparams;

	int fileformat;

	if( iss.Format < 1 ) 
		fileformat = GetSavingFormat(szFileName);
	else 
		fileformat = iss.Format-1;

	float width,height,imgwidth,imgheight,newwidth,newheight;

	Image bm(szFileName);
	imgwidth = bm.GetWidth();
	imgheight = bm.GetHeight();

	width = iss.NewWidth;
	height = iss.NewHeight;

	newwidth=imgwidth;
	newheight=imgheight;


	// Если включена опция "Оставить без изменений", просто копируем имя исходного файла
	if(iss.KeepAsIs) 
		lstrcpy(szBufferImage, szFileName);

	else
	{
		if(iss.SaveProportions)
		{
			if( width && imgwidth > width )
			{
				newwidth = width;
				newheight = newwidth / imgwidth * imgheight;
			}
			else if(height && imgheight > height)
			{
				newheight = height;
				newwidth = newheight/imgheight*imgwidth;
			}
		}
		else
		{
			if(width>0) newwidth=width;
			if(height>0) newheight=height;
		}


		Graphics g(m_hWnd, true);
		g.SetPageUnit(UnitPixel);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		Bitmap BackBuffer(newwidth, newheight, &g);


		Graphics gr(&BackBuffer);
		if(fileformat != 2)
			gr.Clear(Color(255,255,255,255));
		else 
			gr.Clear(Color(125,255,255,255));
		//gr.Clear(Color(255,255,255));
		g.SetPageUnit(UnitPixel);
		gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );





		//gr.FillRectangle(&br,(float)1, (float)1, (float)width, (float)height);
		gr.SetPixelOffsetMode(PixelOffsetModeHalf);
		if(!width && !height)
			gr.DrawImage(/*backBuffer*/&bm, (int)0, (int)0, (int)newwidth,(int)newheight);
		else
			gr.DrawImage(/*backBuffer*/&bm, (int)-1, (int)-1, (int)newwidth+2,(int)newheight+2);


		RectF bounds(0, 0, float(newwidth), float(newheight));

		// Добавляем текст на картинку (если опция включена)
		if(iss.AddText)
		{

			SolidBrush brush(Color(GetRValue(Settings.LogoSettings.TextColor),GetGValue(Settings.LogoSettings.TextColor),GetBValue(Settings.LogoSettings.TextColor)));

			int HAlign[6]={0,1,2,0,1,2};	
			int VAlign[6]={0,0,0,2,2,2};	

			Settings.LogoSettings.Font.lfQuality=Settings.LogoSettings.Font.lfQuality|ANTIALIASED_QUALITY ;
			Font font(/*L"Tahoma", 10, FontStyleBold*/::GetDC(0),&Settings.LogoSettings.Font);

			WCHAR Buffer[256];
			//wsprintf(Buffer,_T("%dx%d "),(int)imgwidth,(int)imgheight);
			// что строка заканчивается нулем    
			SolidBrush brush2(Color(70,0,0,0));

			RectF bounds2(1, 1, float(newwidth), float(newheight)+1);
			DrawStrokedText(gr, Settings.LogoSettings.Text,bounds2,font,MYRGB(255,Settings.LogoSettings.TextColor),MYRGB(180,Settings.LogoSettings.StrokeColor),HAlign[Settings.LogoSettings.TextPosition],VAlign[Settings.LogoSettings.TextPosition], 1);





		}

		// Добавляем логотип на картинку (если опция включена)
		if(iss.AddLogo)
		{

			Bitmap logo(Settings.LogoSettings.FileName);
			if(logo.GetLastStatus()==Ok)
			{
				int x,y;
				int logowidth,logoheight;
				logowidth=logo.GetWidth();
				logoheight=logo.GetHeight();
				if(Settings.LogoSettings.LogoPosition<3) y=0;
				else y=newheight-logoheight;
				if(Settings.LogoSettings.LogoPosition==0||Settings.LogoSettings.LogoPosition==3)
					x=0;
				if(Settings.LogoSettings.LogoPosition==2||Settings.LogoSettings.LogoPosition==5)
					x=newwidth-logowidth;
				if(Settings.LogoSettings.LogoPosition==1||Settings.LogoSettings.LogoPosition==4)
					x=(newwidth-logowidth)/2;


				gr.DrawImage(&logo, (int)x, (int)y,logowidth,logoheight);
			}
		}

		MySaveImage(&BackBuffer,L"image.jpg",szBufferImage,fileformat,iss.Quality);
	} 

	if(iss.KeepAsIs)

	{
		CString Ext = GetFileExt(szFileName);
		if(Ext == _T("png"))
			fileformat = 1;
		else fileformat = 0;

	}
	if(thumbwidth && szBufferThumb)
	{
		// Генерирование превьюшки с шаблоном в отдельной функции
		GenThumb(szBufferImage,&bm, thumbwidth, newwidth, newheight, szBufferThumb, fileformat);
	}

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
	ResultsPanel.EnableMediaInfo(IsLastVideo);
	CancelByUser = false;
	ShowNext();
	ShowPrev();
	MainDlg = (CMainDlg*) WizardDlg->Pages[2];
	Toolbar.CheckButton(IDC_USETEMPLATE,Settings.UseTxtTemplate);
	//SendDlgItemMessage(IDC_USETEMPLATE, BM_SETCHECK, Settings.UseTxtTemplate);
	FileProgress(_T(""), false);
	UrlList.RemoveAll();
	ResultsPanel.Clear();
	//logoparams = WizardDlg->logoparams;
	EnablePrev(false);
	EnableNext();
	EnableExit(false);
	SetNextCaption(TR("Остановить"));

	int code = ResultsPanel.GetCodeType();
	int newcode = code;
	bool Thumbs = Settings.ThumbSettings.CreateThumbs;


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
	ResultsPanel.SetCodeType(newcode);
	TabCtrl_SetCurSel(GetDlgItem(IDC_RESULTSTAB), Settings.CodeLang);
	ResultsPanel.SetPage(Settings.CodeLang);

	::SetFocus(GetDlgItem(IDC_CODEEDIT));
	Start();		//Запускаем процесс загрузки файлов на сервер (отдельный поток)
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
	{
		SetTimer(2, 250); 
	}


	if(!Show)
	{
		::ShowWindow(GetDlgItem(IDC_FILEPROGRESS),Show?SW_SHOW:SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_SPEEDLABEL),Show?SW_SHOW:SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_PERCENTLABEL),Show?SW_SHOW:SW_HIDE);
	}
}



void CUploadDlg::GenThumb(LPCTSTR szImageFileName, Image *bm, int ThumbWidth, int newwidth, int newheight, LPTSTR szBufferThumb, int fileformat)
{
	int FileSize = MyGetFileSize(szImageFileName);
	TCHAR SizeBuffer[100]=_T("\0");
	if(FileSize>0)
		NewBytesToString(FileSize,SizeBuffer,sizeof(SizeBuffer));

	CString ThumbnailText = Settings.ThumbSettings.Text; // Text that will be drawn on thumbnail

	ThumbnailText.Replace(_T("%width%"), IntToStr(newwidth)); //Replacing variables names with their values
	ThumbnailText.Replace(_T("%height%"), IntToStr(newheight));
	ThumbnailText.Replace(_T("%size%"), SizeBuffer);

	int thumbwidth=ThumbWidth;
	if(Settings.ThumbSettings.UseThumbTemplate)
	{
		Graphics g1(m_hWnd);
		Image templ(GetAppFolder()+_T("Data\\thumb.png"));
		int ww = templ.GetWidth();
		CString s;


		Font font(::GetDC(0), &Settings.ThumbSettings.ThumbFont);

		RectF TextRect;

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(_T("test"),-1,&font,PointF(0,0),&TextRect);


		thumbwidth-=4;
		int thumbheight=((float)thumbwidth/(float)newwidth*newheight);


		int LabelHeight=TextRect.Height+1;
		int RealThumbWidth=thumbwidth+4;
		int RealThumbHeight=thumbheight+19;

		Bitmap ThumbBuffer(RealThumbWidth, RealThumbHeight, &g1);
		Graphics thumbgr(&ThumbBuffer);
		thumbgr.SetPageUnit(UnitPixel);
		thumbgr.Clear(Color(255,255,255,255));
		RectF thu((float)(Settings.ThumbSettings.DrawFrame?1:0), (float)(Settings.ThumbSettings.DrawFrame?1:0), (float)thumbwidth,(float)thumbheight);
		thumbgr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );

		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		thumbgr.SetSmoothingMode( SmoothingModeHighQuality);


		thumbgr.SetSmoothingMode(SmoothingModeAntiAlias);
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );

		RectF t((float)0, (float)12, (float)5,(float)RealThumbHeight);

		thumbgr.DrawImage(&templ,t,0,13,4,4,UnitPixel);

		RectF t2((float)ThumbWidth-6, (float)9, (float)6,(float)RealThumbHeight);

		thumbgr.DrawImage(&templ,t2,158,11,6,6,UnitPixel);

		RectF t3((float)6, (float)0, (float)RealThumbWidth-8,(float)6);

		thumbgr.DrawImage(&templ,t3,12,0,7,5,UnitPixel);


		RectF t4((float)0, (float)RealThumbHeight-17, (float)RealThumbWidth,(float)17);

		thumbgr.DrawImage(&templ,t4,71.0,92,4,19,UnitPixel);

		thumbgr.DrawImage(&templ,0.0,0.0,0,0,29,29,UnitPixel);
		thumbgr.DrawImage(&templ,ThumbWidth-6,0.0,164-6,0.0,6,9,UnitPixel);
		thumbgr.DrawImage(&templ,0.0,RealThumbHeight-17,0.0,94.0,70,17,UnitPixel);
		thumbgr.DrawImage(&templ,RealThumbWidth-29,RealThumbHeight-29,135.0,82,29,29,UnitPixel);
		thumbgr.DrawImage(/*backBuffer*/bm,(float)2.0/*item->DrawFrame?1:0-1*/,(float)2.0/*(int)item->DrawFrame?1:0-1*/,(float)thumbwidth,(float)thumbheight);

		thumbgr.SetPixelOffsetMode(PixelOffsetModeHalf);

		if(Settings.ThumbSettings.ThumbAddImageSize) // If we need to draw text on thumbnail
		{
			thumbgr.SetPixelOffsetMode(PixelOffsetModeDefault );
			SolidBrush   br222(MYRGB(179,RGB(255,255,255)));
			RectF TextBounds((float)65, (float)RealThumbHeight-17, (float)RealThumbWidth-65-11,(float)17);

			DrawStrokedText(thumbgr,/* Buffer*/ ThumbnailText,TextBounds,font,MYRGB(179,RGB(255,255,255))/*MYRGB(255,params.ThumbTextColor)*/,MYRGB(90,RGB(0,0,0)/*params.StrokeColor*/),1,1, 1);

		}

		Pen p(MYRGB(255,Settings.ThumbSettings.FrameColor));

		if(Settings.ThumbSettings.ThumbAddImageSize){
			StringFormat format;
			format.SetAlignment(StringAlignmentCenter);
			format.SetLineAlignment(StringAlignmentCenter);
			// Font font(L"Tahoma", 7, FontStyleBold);
			SolidBrush LabelBackground(Color(255,140,140,140));;
	
			int LabelAlpha=(Settings.ThumbSettings.TextOverThumb)?Settings.ThumbSettings.ThumbAlpha:255;
			RectF TextBounds(1,RealThumbHeight-LabelHeight,RealThumbWidth-1,LabelHeight+1);

			WCHAR Buffer[256];
			TCHAR SizeBuffer[100]=_T("\0");
		}

		if(fileformat == 2) 
			fileformat = 0;
		MySaveImage(&ThumbBuffer,_T("thumb"),szBufferThumb,fileformat,93);
	}
	else
	{
		Graphics g1(m_hWnd);
		Font font(/*L"Tahoma", 10, FontStyleBold*/::GetDC(0),&Settings.ThumbSettings.ThumbFont);

		RectF TextRect;

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(_T("test"),-1,&font,PointF(0,0),&TextRect);

		if(Settings.ThumbSettings.DrawFrame)
			thumbwidth-=2;
		int thumbheight=((float)thumbwidth/(float)newwidth*newheight);

		int LabelHeight=TextRect.Height+1;
		int RealThumbWidth=thumbwidth+(Settings.ThumbSettings.DrawFrame?2:0);
		int RealThumbHeight=(Settings.ThumbSettings.DrawFrame?2:0)+thumbheight+((Settings.ThumbSettings.ThumbAddImageSize&&(!Settings.ThumbSettings.TextOverThumb))?LabelHeight:0);

		Bitmap ThumbBuffer(RealThumbWidth, RealThumbHeight, &g1);
		Graphics thumbgr(&ThumbBuffer);
		thumbgr.SetPageUnit(UnitPixel);
		RectF thu((float)(Settings.ThumbSettings.DrawFrame?1:0), (float)(Settings.ThumbSettings.DrawFrame?1:0), (float)thumbwidth,(float)thumbheight);
		thumbgr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		thumbgr.SetSmoothingMode( SmoothingModeHighQuality);
		thumbgr.SetSmoothingMode(SmoothingModeAntiAlias);
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		thumbgr.DrawImage(/*backBuffer*/bm,(float)-0.5f/*item->DrawFrame?1:0-1*/,(float)-0.5/*(int)item->DrawFrame?1:0-1*/,(float)RealThumbWidth+1,(float)thumbheight+1.5);
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHalf);
		Pen p(MYRGB(255,Settings.ThumbSettings.FrameColor));

		if(Settings.ThumbSettings.ThumbAddImageSize)
		{
			StringFormat format;
			format.SetAlignment(StringAlignmentCenter);
			format.SetLineAlignment(StringAlignmentCenter);

			SolidBrush LabelBackground(Color(255,140,140,140));;
			int LabelAlpha=(Settings.ThumbSettings.TextOverThumb)?Settings.ThumbSettings.ThumbAlpha:255;
			DrawGradient(thumbgr,Rect(0,RealThumbHeight-LabelHeight-(Settings.ThumbSettings.DrawFrame?1:0),RealThumbWidth,LabelHeight),MYRGB(LabelAlpha,Settings.ThumbSettings.ThumbColor1), MYRGB(LabelAlpha,Settings.ThumbSettings.ThumbColor2));
			RectF TextBounds(1,RealThumbHeight-LabelHeight,RealThumbWidth-1,LabelHeight+1);

			thumbgr.SetPixelOffsetMode(PixelOffsetModeDefault );
			SolidBrush   br222(MYRGB(255,Settings.ThumbSettings.ThumbTextColor));
			DrawStrokedText(thumbgr, /*Buffer*/ThumbnailText,TextBounds,font,MYRGB(255,Settings.ThumbSettings.ThumbTextColor),MYRGB(90,0,0,0/*params.StrokeColor*/),1,1, 1);
		}

		thumbgr.SetPixelOffsetMode(   PixelOffsetModeHalf);
		thumbgr.SetSmoothingMode(SmoothingModeDefault);
		p.SetAlignment(PenAlignmentInset);

		if(Settings.ThumbSettings.DrawFrame)
			DrawRect(ThumbBuffer,MYRGB(255,Settings.ThumbSettings.FrameColor),Rect(0,0,RealThumbWidth,RealThumbHeight));

		if(fileformat == 2) 
			fileformat = 0;

		// Saving thumbnail (without template)
		MySaveImage(&ThumbBuffer,_T("thumb"),szBufferThumb,fileformat,85);
	}
}

bool CUploadDlg::OnHide()
{
	UrlList.RemoveAll();
	ResultsPanel.Clear();
	Settings.UseTxtTemplate = SendDlgItemMessage(IDC_USETEMPLATE, BM_GETCHECK);
	Settings.CodeType = ResultsPanel.GetCodeType();
	Settings.CodeLang = TabCtrl_GetCurSel(GetDlgItem(IDC_RESULTSTAB));
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

void CUploadDlg::FileProgress(const CString Text, bool ShowPrefix)
{
	CString Temp;
	if(ShowPrefix){ 
		Temp+=TR("Текущий файл:"); Temp+=_T("  ");
	}

	Temp += Text;
	SetDlgItemText(IDC_INFOUPLOAD, Temp);

	bool IsProgressBar = ::IsWindowVisible(GetDlgItem(IDC_FILEPROGRESS)) /*&& PrInfo.Total==0*/;

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
	ResultsPanel.GenerateOutput();
}

LRESULT CUploadDlg::OnTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	int Index = TabCtrl_GetCurSel(GetDlgItem(idCtrl));
	ResultsPanel.SetPage(Index);
	return 0;
}

void CUploadDlg::UploadProgress(int CurPos, int Total, int FileProgress)
{
	SendDlgItemMessage(IDC_UPLOADPROGRESS,PBM_SETPOS,CurPos);
#if  WINVER	>= 0x0700
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