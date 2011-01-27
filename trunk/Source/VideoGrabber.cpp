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

#include "stdafx.h"

#define __AFX_H__ // little hack for avoiding __POSITION type redefinition
#include <objbase.h>
#include <streams.h>
#undef __AFX_H__

#include "VideoGrabber.h"
#include "myimage.h"
#include "mediainfodlg.h"
#include <qedit.h>
#include "fileinfohelper.h"
#include "Core/ImageConverter.h"
#include "LogWindow.h"
#ifdef DEBUG
#define MyInfo(p) SetDlgItemText(IDC_FILEEDIT, p)
#else
#define MyInfo
#endif
#pragma comment(lib, "winmm.lib")

typedef struct tagVIDEOINFOHEADER2 {
    RECT                rcSource;
    RECT                rcTarget;
    DWORD               dwBitRate;
    DWORD               dwBitErrorRate;
    REFERENCE_TIME      AvgTimePerFrame;
    DWORD               dwInterlaceFlags;
    DWORD               dwCopyProtectFlags;
    DWORD               dwPictAspectRatioX; 
    DWORD               dwPictAspectRatioY; 
    union {
        DWORD           dwControlFlags;
        DWORD           dwReserved1;
    };
    DWORD               dwReserved2;
    BITMAPINFOHEADER    bmiHeader;
} VIDEOINFOHEADER2;



// Вспомогательные функции 
// для нахождения входных и выходных пинов DirectShow фильтров
HRESULT GetPin(IBaseFilter * pFilter, PIN_DIRECTION dirrequired,  int iNum, IPin **ppPin);
IPin *  GetInPin ( IBaseFilter *pFilter, int Num );
IPin *  GetOutPin( IBaseFilter *pFilter, int Num );

HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
    CComPtr< IEnumPins > pEnum;
    *ppPin = NULL;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if(FAILED(hr)) 
        return hr;

    ULONG ulFound;
    IPin *pPin;
    hr = E_FAIL;

    while(S_OK == pEnum->Next(1, &pPin, &ulFound))
    {
        PIN_DIRECTION pindir = (PIN_DIRECTION)3;

        pPin->QueryDirection(&pindir);
        if(pindir == dirrequired)
        {
            if(iNum == 0)
            {
                *ppPin = pPin;  // Return the pin's interface
                hr = S_OK;      // Found requested pin, so clear error
                break;
            }
            iNum--;
        } 

        pPin->Release();
    } 

    return hr;
}


IPin * GetInPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
    return pComPin;
}


IPin * GetOutPin( IBaseFilter * pFilter, int nPin )
{
    CComPtr<IPin> pComPin=0;
    GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
    return pComPin;
}


//
// CallBack класс для SampleGrabbera
//
// this object is a SEMI-COM object, and can only be created statically.

class CSampleGrabberCB : public ISampleGrabberCB 
{
	public:
		SENDPARAMS sp;
		CImgSavingThread *SavingThread;
		CVideoGrabber *vg;
		// Эти параметры устанавливаются главным потоком
		long Width;
		long Height;
		bool Grab; // для избавления от дубликатов
		CEvent ImageProcessEvent;
		HANDLE BufferEvent;
		LONGLONG prev, step; // не используется

		// Fake out any COM ref counting
		STDMETHODIMP_(ULONG) AddRef() { return 2; }
		STDMETHODIMP_(ULONG) Release() { return 1; }

		// Fake out any COM QI'ing
		STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
		{
			CheckPointer(ppv,E_POINTER);
	        
			if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) 
			{
				*ppv = (void *) static_cast<ISampleGrabberCB*> ( this );
				return NOERROR;
			}    

			return E_NOINTERFACE;
		}

		// Не используется
		//
		STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
		{
			return 0;
		}


    // Callback ф-ия вызываемая SampleGrabber-ом, в другом потоке
    //
		STDMETHODIMP BufferCB( double SampleTime, BYTE * pBuffer, long BufferSize )
		{
			if(!Grab) return 0; //Контроль ложноых вызовов
			TCHAR szFilename[MAX_PATH];

			LONGLONG time=SampleTime * 10000000;
			prev=time;
			TCHAR   buf[256];
			wsprintf(buf, TEXT("%02d:%02d:%02d"),int(SampleTime/3600),(int)(long(SampleTime)/60)%60, (int)long(long(SampleTime)%60) /*,long(SampleTime/100)*/);




			BITMAPFILEHEADER bfh;
			memset( &bfh, 0, sizeof( bfh ) );
			bfh.bfType = 'MB';
			bfh.bfSize = sizeof( bfh ) + BufferSize + sizeof( BITMAPINFOHEADER );
			bfh.bfOffBits = sizeof( BITMAPINFOHEADER ) + sizeof( BITMAPFILEHEADER );

			DWORD Written = 0;

			// Write the bitmap format

			BITMAPINFOHEADER bih;
			memset( &bih, 0, sizeof( bih ) );
			bih.biSize = sizeof( bih );
			bih.biWidth = Width;
			bih.biHeight = Height;
			bih.biPlanes = 1;
			bih.biBitCount = 24;

			BITMAPINFO bi;
			bi.bmiHeader=bih;


			sp.bi=bi;
			sp.pBuffer=pBuffer;
			sp.szTitle=buf;
			sp.BufSize =  BufferSize;
			sp.vg = vg;
			SavingThread->Save(sp);
			Grab = false;
			return 0;
		}
};


//      Класс CVideoGrabber
//

CVideoGrabber::CVideoGrabber()
{
	Terminated = true;
}

CVideoGrabber::~CVideoGrabber()
{
	*m_szFileName=0;
}

//  Принимаем имя файла из главного окна
//
bool CVideoGrabber::SetFileName(LPCTSTR FileName)
{
	lstrcpy(m_szFileName,FileName);
	// Заносим в текстовое поле имя файла, полученное от главного окна
	SetDlgItemText(IDC_FILEEDIT,m_szFileName);
	return false;
}

LRESULT CVideoGrabber::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	PageWnd=m_hWnd;

	// Установка интервалов UpDown контролов
	SendDlgItemMessage(IDC_UPDOWN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1) );
	SendDlgItemMessage(IDC_QUALITYSPIN,UDM_SETRANGE,0,(LPARAM) MAKELONG((short)100, (short)1) );
		
	SetDlgItemInt(IDC_NUMOFFRAMESEDIT,Settings.VideoSettings.NumOfFrames);
	SetDlgItemInt(IDC_QUALITY,Settings.VideoSettings.JPEGQuality);

	TRC(IDC_EXTRACTFRAMES,"Извлечение видео кадров");
	TRC(IDC_SELECTVIDEO,"Обзор...");

	TRC(IDC_PATHTOFILELABEL,"Имя файла:");
	TRC(IDCANCEL,"Остановить");
	TRC(IDC_DEINTERLACE,"Деинтерлейсинг");
	TRC(IDC_FRAMELABEL,"Кол-во кадров:");
	TRC(IDC_MULTIPLEFILES,"множество файлов");
	TRC(IDC_SAVEASONE,"один файл");
	TRC(IDC_SAVEAS,"Сохранить как:");
	TRC(IDC_GRAB,"Извлечь");
	TRC(IDC_QUALITYLABEL,"Качество:");
	TRC(IDC_GRABBERPARAMS,"Параметры...");
	TRC(IDC_FILEINFOBUTTON,"Информация о файле");
	// Заносим в текстовое поле имя файла, полученное от главного окна
	SetDlgItemText(IDC_FILEEDIT,m_szFileName);

	bool check=true;
	// Установка режима сохранения
	SendDlgItemMessage(IDC_MULTIPLEFILES,BM_SETCHECK,check);
	SendDlgItemMessage(IDC_SAVEASONE,BM_SETCHECK,!check);

	::ShowWindow(GetDlgItem(IDC_STOP),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_PROGRESSBAR),SW_HIDE);
	SavingMethodChanged();
	ThumbsView.SubclassWindow(GetDlgItem(IDC_THUMBLIST));
	ThumbsView.Init();

	return 1;  // Let the system set the focus
}

LRESULT CVideoGrabber::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if(!Terminated)
	{
		SignalStop();				//  Посылаем потоку граббинга сигнал останова
		if(!IsStopTimer)
		{
			TimerInc = 8;				// Ждем 8 секунд, прежде чем убиваем поток
			SetTimer(1, 1000, NULL);
			IsStopTimer = true;
		}
		else 
		{
			CanceledByUser=true;
			Terminate();		// Убиваем поток
			ThreadTerminated();		
		}

	}

	return 0;
}

LRESULT CVideoGrabber::OnBnClickedGrab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	WizardDlg->LastVideoFile = IU_GetWindowText(GetDlgItem(IDC_FILEEDIT));
	RECT WindowRect,ClientRect;
	
	Terminated = false;
	IsStopTimer=false;

	::ShowWindow(GetDlgItem(IDC_FRAMELABEL),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_DEINTERLACE),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_NUMOFFRAMESEDIT),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_UPDOWN),SW_HIDE);
	
	::ShowWindow(GetDlgItem(IDCANCEL),SW_SHOW);
	
	::EnableWindow(GetDlgItem(IDC_GRAB),0);
	::EnableWindow(GetDlgItem(IDC_FILEEDIT),0);
	::EnableWindow(GetDlgItem(IDC_SELECTVIDEO),0);
	::ShowWindow(GetDlgItem(IDC_PROGRESSBAR),SW_SHOW);
	SetNextCaption(TR("Далее >"));
	EnableNext(false);
	EnablePrev(false);
	EnableExit(false);
	TRC(IDC_STOP,"Остановить");
	
	NumOfFrames=GetDlgItemInt(IDC_NUMOFFRAMESEDIT);
	if(!NumOfFrames) NumOfFrames=5;
	SendDlgItemMessage(IDC_PROGRESSBAR,PBM_SETPOS,0);
	SendDlgItemMessage(IDC_PROGRESSBAR,PBM_SETRANGE,0,MAKELPARAM(0, NumOfFrames));
	CanceledByUser = false;

	m_hThread=NULL;
	this->Start();
	
	return 0;
}

DWORD CVideoGrabber::Run()
{
	::CoInitialize(NULL);
	TCHAR buffer[256];
	GetDlgItemText(IDC_FILEEDIT, buffer, 256);
	if(lstrlen(buffer) < 1) return 0;

	int res = GrabBitmaps(buffer);
	if(res < 0)
		WriteLog(logError, TR("Модуль извлечения кадров"), ErrorStr, CString(TR("File:"))+_T("  ")+buffer+_T("\n")+TR("Ошибка при извлечении кадров.")/*+_T("\n")*/);
	::CoUninitialize();
	return ThreadTerminated();
}

bool CVideoGrabber::OnAddImage(SENDPARAMS *sp)
{
	if(!sp) return 0;
	BYTE *pBuffer = sp->pBuffer;
	BITMAPINFO bi = sp->bi;
	if(bi.bmiHeader.biWidth > 10000) return 0; 
	if(bi.bmiHeader.biHeight > 10000) return 0; 
	if(CanceledByUser) return 0;
	GrabInfo(CString(TR("Извлекаю кадр "))+sp->szTitle);
	if(!pBuffer)return 0;
	Bitmap bm(&bi,pBuffer);

	if(bm.GetLastStatus()!=Ok) return 0;
		CString fileNameBuffer;

	if(SendDlgItemMessage(IDC_DEINTERLACE,BM_GETCHECK)==BST_CHECKED)
	{
		// Genial deinterlace realization ;)
		int iwidth = bm.GetWidth();
		int iheight = bm.GetHeight();
		int halfheight = iheight/2;
		Graphics g(m_hWnd,true);
		Bitmap BackBuffer (iwidth,  halfheight, &g);
		Graphics gr(&BackBuffer);
		gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );
		gr.DrawImage(&bm,0,0,iwidth,halfheight);
		Graphics gr2(&bm);
		gr2.SetInterpolationMode(InterpolationModeHighQualityBicubic );
		gr2.DrawImage(&BackBuffer,0,0,iwidth,iheight);
	}

	MySaveImage(&bm, _T("grab"), fileNameBuffer, 1, 100);
	ThumbsView.AddImage(fileNameBuffer, sp->szTitle, &bm);
	return true;  
}

bool CVideoGrabber::GrabInfo(LPCTSTR String)
{
	ErrorStr = String;
	SetDlgItemText(IDC_GRABINFOLABEL,String);
	return false;
}


int CVideoGrabber::ThreadTerminated()
{
	Terminated = true;
	KillTimer(1);
	IsStopTimer=false;

	::EnableWindow(GetDlgItem(IDC_GRAB),1);

	::ShowWindow(GetDlgItem(IDC_STOP),SW_HIDE);
	::ShowWindow(GetDlgItem(IDC_PROGRESSBAR),SW_HIDE);
	if(CanceledByUser)
		GrabInfo(TR("Извлечение кадров было остановлено пользователем."));
	SavingThread.Stop();
	SavingThread.Reset();

	::ShowWindow(GetDlgItem(IDC_FRAMELABEL),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_DEINTERLACE),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_NUMOFFRAMESEDIT),SW_SHOW);
	::ShowWindow(GetDlgItem(IDC_UPDOWN),SW_SHOW);
	CheckEnableNext();
	::EnableWindow(GetDlgItem(IDC_FILEEDIT),1);
	::EnableWindow(GetDlgItem(IDC_SELECTVIDEO),1);

	EnablePrev();
	EnableExit();
	return 0;
}

 LRESULT CVideoGrabber::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 {
	 TimerInc--;
	 TCHAR szBuffer[256];
	 if(TimerInc>0)
	 {
	 wsprintf(szBuffer,CString(TR("Остановить"))+_T(" (%d)"),TimerInc);
	 SetDlgItemText(IDC_STOP,szBuffer);
	 }
	 else
	 {
		 Terminate();
		ThreadTerminated();
	 }
	return 0;
 }
LRESULT CVideoGrabber::OnBnClickedGrabberparams(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CSettingsDlg dlg(3);
	dlg.DoModal(m_hWnd);
	return 0;
}

LRESULT CVideoGrabber::OnBnClickedMultiplefiles(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	SavingMethodChanged();
	return 0;
}

void CVideoGrabber::SavingMethodChanged(void)
{
	BOOL check = SendDlgItemMessage(IDC_MULTIPLEFILES,BM_GETCHECK);
	::EnableWindow(GetDlgItem(IDC_GRABBERPARAMS),!check);
}
  
int CVideoGrabber::GenPicture(CString &outFileName)
{
	RectF TextRect;
	int infoHeight = 0;
	CString Report;

	if(Settings.VideoSettings.ShowMediaInfo)
	{
		TCHAR buffer[256];
		GetDlgItemText(IDC_FILEEDIT, buffer, 256);
		bool bMediaInfoResult = GetMediaFileInfo(buffer, Report);

		Graphics g1(m_hWnd);
		
		CString s;

		Font font(::GetDC(0), &Settings.VideoSettings.Font);

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(Report, -1, &font, PointF(0,0), &TextRect);
		infoHeight = TextRect.Height;
	}

	int n = ThumbsView.GetItemCount();
	int ncols = min(Settings.VideoSettings.Columns,n);
	int nstrings = n/ncols+((n%ncols)?1:0);
	int maxwidth = ThumbsView.maxwidth;
	int maxheight = ThumbsView.maxheight;
	int gapwidth = Settings.VideoSettings.GapWidth;
	int gapheight = Settings.VideoSettings.GapHeight;
	infoHeight += gapheight;
	int tilewidth = Settings.VideoSettings.TileWidth;
	int tileheight =(int)((float)tilewidth)/((float)maxwidth)*((float)maxheight);
	int needwidth = gapwidth+ncols*(tilewidth+gapwidth);
	int needheight = gapheight+nstrings*(tileheight+gapheight) + infoHeight;

	
	RECT rc;
	GetClientRect(&rc);

	Bitmap *BackBuffer;
	Graphics g(m_hWnd,true);
	BackBuffer = new Bitmap(needwidth, needheight, &g);
	
	Graphics gr(BackBuffer);
	
	Image *bm=NULL;
	Rect r(0,0,needwidth,needheight);
	gr.Clear(Color(255,180,180,180));
	LinearGradientBrush br(r, Color(255, 224, 224, 224), Color(255, 243, 243, 243), 
            LinearGradientModeBackwardDiagonal); 


	gr.FillRectangle(&br,r);


	int x,y;
	Pen Framepen(Color(90,90,90));
	TCHAR buf[256]=_T("\0");
	Font font(L"Tahoma", 10, FontStyleBold);
	Color ColorText(140,255,255,255);Color ColorStroke(120,0,0,0);

	for(int i=0; i<n; i++)
	{
		bm=new Image(ThumbsView.GetFileName(i));
		x=gapwidth+(i%ncols)*(tilewidth+gapwidth);
		y=infoHeight + (infoHeight? gapheight:0)+((i/ncols))*(tileheight+gapheight);
		ThumbsView.GetItemText(i,0,buf,256);
		gr.DrawImage(bm, (int)(x/*(tilewidth-newwidth)/2*/), (int)y, (int)tilewidth,(int)tileheight);
		DrawStrokedText(gr, buf,RectF(x/*(tilewidth-newwidth)/2*/,y, tilewidth,tileheight),font,ColorText,ColorStroke,3,3);
		gr.DrawRectangle(&Framepen,Rect(x/*(tilewidth-newwidth)/2*/,(int)y, (int)tilewidth,(int)tileheight));
		if(bm) delete bm;
	}
	
	if(infoHeight)
	{	
		 StringFormat format;
		format.SetAlignment(StringAlignmentNear);
		format.SetLineAlignment(StringAlignmentNear);
		Font font(::GetDC(0), &Settings.VideoSettings.Font);
		//Font font(L"Arial", 12, FontStyleBold);
		SolidBrush br(/*Settings.ThumbSettings.ThumbTextColor*/MYRGB(255, Settings.VideoSettings.TextColor));
		RectF textBounds(gapwidth, gapheight, needwidth-gapwidth, infoHeight-gapheight);
		gr.DrawString(Report, -1, &font, textBounds, &format, &br);
		///DrawStrokedText(gr, Report,textBounds,font,ColorText,ColorStroke,3,3);

	}

	MySaveImage(BackBuffer,_T("grab_custom"),outFileName,1,100);
	if(BackBuffer) delete BackBuffer;
	  return 0;
}

LRESULT CVideoGrabber::OnBnClickedButton1(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFileDialog fd(true,0,0,4|2,VIDEO_DIALOG_FORMATS,m_hWnd);
	
	if(fd.DoModal() != IDOK || !fd.m_szFileName) return 0;

	SetDlgItemText(IDC_FILEEDIT,fd.m_szFileName);
	
	return 0;
}


//////////////////////////////////////

CImgSavingThread::CImgSavingThread()
{
	StopEvent.Create();
	SavingEvent.Create();
	ImageProcessEvent.Create();
}

void CImgSavingThread::Reset()
{
	StopEvent.ResetEvent();
	SavingEvent.ResetEvent();
	ImageProcessEvent.ResetEvent();
}
void CImgSavingThread::Save(SENDPARAMS sp)
{
	if(IsRunning())
	{
	#ifdef DEBUG
		vg->SetDlgItemText(IDC_FILEEDIT,_T("CImgSavingThread::Save DataCriticalSection.Lock()"));
	#endif
		DataCriticalSection.Lock();
		m_sp = sp;
		DataCriticalSection.Unlock();
		#ifdef DEBUG
		//vg->GrabInfo(_T("WaitForEvent(); ..."));
		#endif
		SavingEvent.SetEvent();
		ImageProcessEvent.WaitForEvent();
		ImageProcessEvent.ResetEvent();
#ifdef DEBUG	
		//vg->GrabInfo(_T("WaitForEvent() finished; ..."));
#endif
	}
}

DWORD CImgSavingThread::Run()
{
	HANDLE EvArray[2]={StopEvent, SavingEvent};
	DWORD EventIndex = -1;
	do
	{
		DWORD Res = WaitForMultipleObjects(2, EvArray, FALSE, INFINITE);

		if(Res == WAIT_FAILED) continue;

		EventIndex = Res - WAIT_OBJECT_0;
		if(EventIndex == 1)
		{
			SavingEvent.ResetEvent();

			BOOL b;
			DataCriticalSection.Lock();
	#ifdef DEBUG
			vg->GrabInfo(_T("Saving image(); ..."));
	#endif
			m_sp.vg->OnAddImage(&m_sp);
			#ifdef DEBUG
			vg->GrabInfo(_T("saving finished(); ..."));
			#endif
#ifdef DEBUG
			vg->GrabInfo(_T("DataCriticalSection.Unlock(); ..."));
#endif
			DataCriticalSection.Unlock();
#ifdef DEBUG
			vg->GrabInfo(_T("SetEvent(); ..."));
#endif
			ImageProcessEvent.SetEvent();
		}
	}
	while(EventIndex!=0);
	ImageProcessEvent.SetEvent();
	ATLTRACE(_T("CImgSavingThread FINISHED!"));
	return 0;
}

void CImgSavingThread::Stop()
{
	if(!IsRunning()) return;
	StopEvent.PulseEvent();
	WaitForThread();
}

CImgSavingThread::~CImgSavingThread()
{
	Stop();
}

int CVideoGrabber::GrabBitmaps(TCHAR * szFile )
{
	USES_CONVERSION;
	bool IsWMV=false;
	bool IsOther=false;
	CComPtr< ISampleGrabber > pGrabber;
	CComPtr< IBaseFilter >    pSource;
	CComPtr< IBaseFilter >    pASF;
	CComPtr< IGraphBuilder >  pGraph;
	CComPtr< IVideoWindow >   pVideoWindow;
	HRESULT hr;

	if (!szFile)
		return -1;
	TCHAR szBuffer[256];
	LPCTSTR szFileName=myExtractFileName(szFile);
	if(szFileName)
	{
		wsprintf(szBuffer,CString(TR("Извлечение кадров из видео файла")) + _T(" \"%s\" ..."),(LPCTSTR)szFileName);
		GrabInfo(szBuffer);
	}

	IsOther = true;
	bool IsAVI = IsStrInList(GetFileExt(szFileName), _T("avi\0\0"));
	IsWMV = IsStrInList(GetFileExt(szFileName), _T("wmv\0asf\0\0"));

	if(!IsWMV)
		IsOther=IsStrInList(GetFileExt(szFileName),_T("mkv\0wmv\0\mpg\0"));

	// Create the sample grabber
	//
	pGrabber.CoCreateInstance( CLSID_SampleGrabber );
	if( !pGrabber )
	{
		GrabInfo(_T("Could not create object CLSID_SampleGrabber."));
		return -1;
	}
	CComQIPtr< IBaseFilter, &IID_IBaseFilter > pGrabberBase( pGrabber );

	// Create the file reader
	//
	//CLSID_WMAsfReader
	if(!IsOther)
	{
		pSource.CoCreateInstance( CLSID_AsyncReader  );
		if( !pSource )
		{
			GrabInfo(_T("Couldn't Create source filter."));
			return -1;
		}
	}
	if(IsWMV){
		pASF.CoCreateInstance( CLSID_WMAsfReader  );
		if( !pASF )
		{
			GrabInfo( TEXT("An error occured while creating WMV Reader filter.") );
			return -1;
		}
	}
	// Create the graph
	//
	pGraph.CoCreateInstance( CLSID_FilterGraph );
	if( !pGraph )
	{
		_tprintf( TEXT("Could not not create the graph!\r\n") );
		return -1;
	}
	bool Error=false;
	CComPtr< IGraphBuilder >  pGraph2;
	if(!IsAVI)
	{
		pGraph2.CoCreateInstance( CLSID_FilterGraph );


		GrabInfo( TR("Поиск необходимых кодеков...") );
		CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow2 = pGraph2;
		if (pWindow2)
		{
			hr = pWindow2->put_AutoShow(OAFALSE);
		}

		hr = pGraph2->RenderFile(szFile,NULL);
		if( FAILED( hr ) )
		{
			GrabInfo( TR("Невозможно подобрать кодеки (формат не поддерживается).") ); 
			Error = true;
		}
		CComQIPtr< IMediaControl, &IID_IMediaControl > pControl2( pGraph2);
		pControl2->Stop();
		pGraph2->Abort();

	}
	// Put them in the graph
	//
	if(IsWMV)
		hr = pGraph->AddFilter( pASF, L"Source" );//my
	else if(IsOther)
		hr = pGraph->AddSourceFilter( szFile, NULL, &pSource);//my
	else
		hr = pGraph->AddFilter( pSource, L"Source" );

	if( FAILED( hr ) )
	{
		GrabInfo( TR("Невозможно загрузить кодек.") );  
		return -1;
	}

	CMediaType GrabType;
	GrabType.SetType( &MEDIATYPE_Video );
	GrabType.SetSubtype( &MEDIASUBTYPE_RGB24 );
	hr = pGrabber->SetMediaType( &GrabType );

	hr = pGraph->AddFilter( pGrabberBase, L"Grabber" );

	// Load the source
	//
	if(IsWMV)
	{
		CComQIPtr< IFileSourceFilter, &IID_IFileSourceFilter > pLoad( pASF);
		hr = pLoad->Load( T2W( szFile ), NULL );
	}
	else if(!IsOther){

		CComQIPtr< IFileSourceFilter, &IID_IFileSourceFilter > pLoad( /*pASF*/ pSource);
		if(!Error)
			GrabInfo( TR("Загрузка файла...") );  
		hr = pLoad->Load( T2W( szFile ), NULL );
	}
	if( FAILED( hr ) )
	{
		GrabInfo( TR("Невозможно загрузить видео файл.") );
		return -1;
	}

	// Tell the grabber to grab 24-bit video. Must do this
	// before connecting it
	//
	/* CMediaType GrabType;
	GrabType.SetType( &MEDIATYPE_Video );
	GrabType.SetSubtype( &MEDIASUBTYPE_RGB24 );*/
	//GrabType.SetSubtype( &MEDIASUBTYPE_AYUV );*/

	//    hr = pGrabber->SetMediaType( &GrabType );

	if( FAILED( hr ) )
	{
		GrabInfo( _T("Unable to set MEDIASUBTYPE_RGB24.") );
		return -1;
	}

	// Get the output pin and the input pin
	//
	CComPtr< IPin > pSourcePin;
	CComPtr< IPin > pGrabPin;
	CSampleGrabberCB CB;
	if(IsWMV)
		pSourcePin = GetOutPin( pASF, 1 );
	else
		pSourcePin = GetOutPin( pSource, 0 );
	pGrabPin   = GetInPin( pGrabberBase, 0 );

	// ... and connect them
	//
	if(!Error)
		GrabInfo( TR("Подключение кодеков...") ); 
	else  GrabInfo( TR("Ещё одна попытка подключения кодеков...") ); 

	AM_MEDIA_TYPE mt2;
	hr = pGrabber->GetConnectedMediaType( &mt2 );

	hr = pGraph->Connect( pSourcePin, pGrabPin );

	if( FAILED( hr ) )
	{
		GrabInfo(  TR("Ошибка соединения фильтров (формат не поддерживается).") );
		return -1;
	}

	// This semi-COM object will receive sample callbacks for us
	//
	CB.vg=this;

	CB.SavingThread = &SavingThread;
	CB.BufferEvent = CreateEvent(0, FALSE, FALSE, 0);

	// Ask for the connection media type so we know its size
	//
	AM_MEDIA_TYPE mt;
	hr = pGrabber->GetConnectedMediaType( &mt );
	if(FAILED( hr ))
	{
		GrabInfo(  TEXT("Unable to determine what we connected.") );
		return -1; 
	}
	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mt.pbFormat;
	if(!FAILED( hr ))
	{
		CB.Width  = vih->bmiHeader.biWidth;
		CB.Height = vih->bmiHeader.biHeight;
		FreeMediaType( mt );
	}

	//GrabInfo( _T("Trying to get out pin") );  

	// Render the grabber output pin (to a video renderer)
	//

	CComPtr <IPin> pGrabOutPin = GetOutPin( pGrabberBase, 0 );
	//GrabInfo( _T("Trying to render graph.") );  
	hr = pGraph->Render( pGrabOutPin );
	if( FAILED( hr ) )
	{
		_tprintf( TEXT("Error while receiving data from filter's output pins.\r\n") );
		return -1;
	}



	// Don't buffer the samples as they pass through
	//
	hr = pGrabber->SetBufferSamples( FALSE );

	// Only grab one at a time, stop stream after
	// grabbing one sample
	//
	hr = pGrabber->SetOneShot( TRUE );

	// Set the callback, so we can grab the one sample
	//
	hr = pGrabber->SetCallback( &CB, 1 );
	SavingThread.vg=this;
	SavingThread.Start();
	// Get the seeking interface, so we can seek to a location
	//
	CComQIPtr< IMediaSeeking, &IID_IMediaSeeking > pSeeking( pGraph );

	// Query the graph for the IVideoWindow interface and use it to
	// disable AutoShow.  This will prevent the ActiveMovie window from
	// being displayed while we grab bitmaps from the running movie.
	CComQIPtr< IVideoWindow, &IID_IVideoWindow > pWindow = pGraph;
	if (pWindow)
	{
		hr = pWindow->put_AutoShow(OAFALSE);
	}

	// Find a limited number of frames
	LONGLONG duration;
	pSeeking->GetDuration(&duration);
	if(duration == 0)
	{
		GrabInfo(TR("Не могу определить длину видео потока. Возможно файл поврежден или формат не поддерживается."));
		return 0;
	}

	LONGLONG step = duration/NumOfFrames;
	CHAR buf[256];

	CB.step = step;
	CComQIPtr< IMediaControl, &IID_IMediaControl > pControl( pGraph );
	CComQIPtr< IMediaEvent, &IID_IMediaEvent > pEvent( pGraph );

	long EvCode = 0;

	long EventCode = 0, Param1 = 0, Param2 = 0;

	for( int i = 0 ; i < NumOfFrames ; i++ )
	{
		if(ShouldStop()) 
		{
			CanceledByUser = true;
			break;
		}
		// set position  

		REFERENCE_TIME Start = (i+1)*step-step/5*3; //**/(duration/NUM_FRAMES_TO_GRAB);//** UNITS*40*/;
		hr = pGrabber->SetOneShot( TRUE );
		hr = pSeeking->SetPositions( &Start, AM_SEEKING_AbsolutePositioning|AM_SEEKING_SeekToKeyFrame, 
			0, AM_SEEKING_NoPositioning);
		CB.Grab=true;
		hr = pControl->Run( );
		hr = pEvent->WaitForCompletion( INFINITE, &EvCode );
		SendDlgItemMessage(IDC_PROGRESSBAR, PBM_SETPOS, i+1);
	}
	pControl->Stop();

	if(!CanceledByUser)
	{
		GrabInfo(TR("Извлечение кадров было завершено."));
	}

	return 0;
}

bool CVideoGrabber::OnShow()
{
	SetNextCaption(TR("Извлечь"));
	SetDlgItemText(IDC_FILEEDIT,m_szFileName);
	::ShowWindow(GetDlgItem(IDC_FILEINFOBUTTON), (*MediaInfoDllPath)?SW_SHOW:SW_HIDE);
	EnableNext(true);
	ShowPrev();
	ShowNext();
	EnablePrev();
	EnableExit();
	ThumbsView.MyDeleteAllItems();
	::SetFocus(GetDlgItem(IDC_GRAB));	
	return true;
}
void CVideoGrabber::CheckEnableNext()
{
	EnableNext(ThumbsView.GetItemCount()>1);
}

bool CVideoGrabber::OnNext()
{
	int n=ThumbsView.GetItemCount();
	if(n<1) 
	{
		SendDlgItemMessage(IDC_GRAB, BM_CLICK);
		return false;
	}
	LPCTSTR filename;
	
	WizardDlg->CreatePage(2);
	CMainDlg* MainDlg=(CMainDlg*)WizardDlg->Pages[2];
	
	BOOL check =SendDlgItemMessage(IDC_MULTIPLEFILES,BM_GETCHECK);

	if(check) // If option "Multiple files" turn on
	{
		for(int i=0;i<n;i++)
		{
			filename = ThumbsView.GetFileName(i);
			if(!filename) continue;
			MainDlg->AddToFileList(filename);
		}
	}
	else
	{
		CString outFileName;
		GenPicture(outFileName);
		if(!outFileName.IsEmpty())
		MainDlg->AddToFileList(outFileName);
	}

	ThumbsView.MyDeleteAllItems();
	BOOL scheck = SendDlgItemMessage(IDC_MULTIPLEFILES,BM_GETCHECK);
	 
	Settings.VideoSettings.NumOfFrames=GetDlgItemInt(IDC_NUMOFFRAMESEDIT);

	return true;
}

LRESULT CVideoGrabber::OnLvnItemDelete(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
{
	// В случае опустошения списка деактивируем кнопку "Далее"
	CheckEnableNext();
	return 0;
}
	
LRESULT CVideoGrabber::OnBnClickedFileinfobutton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CMediaInfoDlg dlg;
	TCHAR buffer[256];
	GetDlgItemText(IDC_FILEEDIT, buffer, 256);

	dlg.ShowInfo(buffer);
	return 0;
}
