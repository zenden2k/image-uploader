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

#include "DirectshowFrameGrabber.h"

#include <Core/Utils/CoreUtils.h>
#define __AFX_H__ // little hack for avoiding __POSITION type redefinition
#include <objbase.h>
#include <streams.h>
#undef __AFX_H__
#include <qedit.h>
#include <comdef.h>
#include <comip.h>
#include <Strmif.h>
#include <atlbase.h>
#include "AbstractVideoFrame.h"
#include <Core/Logging.h>
#include <windows.h>
#include <Core/3rdpart/dxerr.h>
#include <zthread/Mutex.h>
#include <zthread/Guard.h>
#include <zthread/Condition.h>
#include "DirectShowUtil.h"
#define tr(arg) (arg)
class DirectshowVideoFrame: public AbstractVideoFrame {
public :
    DirectshowVideoFrame(unsigned char *data, unsigned int dataSize, int64_t time, int width, int height) {
        time_ = time;

        BITMAPFILEHEADER bfh;
        memset( &bfh, 0, sizeof(bfh) );
        bfh.bfType = 'MB';
        bfh.bfSize = sizeof(bfh) + dataSize + sizeof(BITMAPINFOHEADER);
        bfh.bfOffBits = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);

        //DWORD Written = 0;

        // Write the bitmap format

        BITMAPINFOHEADER bih;
        memset( &bih, 0, sizeof(bih) );
        bih.biSize = sizeof(bih);
        bih.biWidth = width;
        bih.biHeight = height;
        bih.biPlanes = 1;
        bih.biBitCount = 24;

        BITMAPINFO bi;
        bi.bmiHeader = bih;

        int dataOffset = sizeof(bfh ) + sizeof(bih);
         dataSize_ = dataSize +  dataOffset;
        data_ = new unsigned char[dataSize + dataOffset ];
        memcpy( data_, &bfh, sizeof(bfh));
        memcpy( data_ + sizeof(bfh), &bih, sizeof(bih));
        memcpy(data_ + dataOffset , data, dataSize);

        width_ = width;
        height_ = height;
    }
    ~DirectshowVideoFrame() {
        delete[] data_;
    }

    bool saveToFile(const Utf8String& fileName) const {
		AbstractImage* image = AbstractImage::createImage();
		if ( !image ) {
			return false;
		} 
		if ( !image->loadFromRawData(AbstractImage::dfBitmapRgb, width_, height_,data_, dataSize_) ) {
			return false;
		}
		return image->saveToFile(fileName);
    }

    AbstractImage* toImage() const  {
		AbstractImage* image = AbstractImage::createImage();
		if ( !image ) {
			return 0;
		} 
		if ( !image->loadFromRawData(AbstractImage::dfBitmapRgb, width_, height_,data_, dataSize_) ) {
			delete image;
		}
        return image;
    }

 protected:
    unsigned char *data_;
    unsigned int dataSize_;
};

inline std::string GetMessageForHresult(HRESULT hr) { 

	//_com_error error(hr); 
	CString cs; 
	WCHAR descr[1024]=L"";
	DXGetErrorDescriptionW(hr,descr, ARRAY_SIZE(descr));
	cs.Format(_T("\r\nError 0x%08x: %s\r\n%s"), hr, DXGetErrorStringW(hr), descr); 
	return IuCoreUtils::WstringToUtf8((LPCTSTR)cs); 
} 



typedef struct tagVIDEOINFOHEADER2
{
    RECT rcSource;
    RECT rcTarget;
    DWORD dwBitRate;
    DWORD dwBitErrorRate;
    REFERENCE_TIME AvgTimePerFrame;
    DWORD dwInterlaceFlags;
    DWORD dwCopyProtectFlags;
    DWORD dwPictAspectRatioX;
    DWORD dwPictAspectRatioY;
    union
    {
        DWORD dwControlFlags;
        DWORD dwReserved1;
    };
    DWORD dwReserved2;
    BITMAPINFOHEADER bmiHeader;
} VIDEOINFOHEADER2;

// некий COM-object для передачи в callback-функция GraphBulder, что-бы не грузить субтитры 
class NoDirectVobSub : public IAMGraphBuilderCallback
{
	int Refs;

public:
	NoDirectVobSub() { Refs = 0; }

	virtual STDMETHODIMP QueryInterface(REFIID riid, __deref_out void** ppv)
	{
		if (riid == IID_IUnknown)                        (*ppv) = static_cast<IUnknown*>(this);
		else if (riid == IID_IAMGraphBuilderCallback)    (*ppv) = static_cast<IAMGraphBuilderCallback*>(this);

		else { (*ppv)=0; return E_NOINTERFACE; }

		AddRef();
		return S_OK;
	}

	// Fake out any COM ref counting
	STDMETHODIMP_(ULONG) AddRef() {
		return 2;
	}
	STDMETHODIMP_(ULONG) Release() {
		return 1;
	}

	virtual HRESULT STDMETHODCALLTYPE SelectedFilter( /* [in] */ IMoniker* pMon)
	{
		HRESULT ret = S_OK;
		//CLSID id; pMon->GetClassID( &id ); // получает всегда CLSID_DeviceMoniker - по нему не проверишь

		IBindCtx* bind;    ;
		if (SUCCEEDED(CreateBindCtx( 0, &bind )))
		{
			LPOLESTR name;
			if (SUCCEEDED(pMon->GetDisplayName( bind, 0, &name )))
			{
				// thanks to http://rsdn.ru/forum/media/4970888.1
				if (wcsstr((wchar_t*)name, L"93A22E7A-5091-45EF-BA61-6DA26156A5D0" )!=0) ret = E_ABORT;    // DirectVobSub
				if (wcsstr((wchar_t*)name, L"9852A670-F845-491B-9BE6-EBD841B8A613" )!=0) ret = E_ABORT;    // DirectVobSub autoload
			}
			bind->Release(); 
		}

		return ret;
	}

	virtual HRESULT STDMETHODCALLTYPE CreatedFilter( /* [in] */ IBaseFilter* pFil) { return S_OK; }
};



class CSampleGrabberCB : public ISampleGrabberCB
{
public:
	CSampleGrabberCB() :condition(mutex) {
	}
	ZThread::Mutex mutex;
	ZThread::Condition condition;
    DirectshowFrameGrabberPrivate *directShowPrivate;
    //CImgSavingThread* SavingThread;
  //  CVideoGrabberPage* vg;
    // Эти параметры устанавливаются главным потоком
    long Width;
    long Height;
    bool Grab; // для избавления от дубликатов
    //CEvent ImageProcessEvent;
    HANDLE BufferEvent;
    LONGLONG prev; // не используется

    // Fake out any COM ref counting
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // Fake out any COM QI'ing
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP SampleCB( double SampleTime, IMediaSample* pSample );


    STDMETHODIMP BufferCB( double SampleTime, BYTE* pBuffer, long BufferSize );

};

class DirectshowFrameGrabberPrivate {
public:
	DirectshowFrameGrabberPrivate() {
		currentFrame_ = NULL;
	}

	~DirectshowFrameGrabberPrivate() {
		//pGraph.Detach();
	}

	DirectshowVideoFrame* currentFrame() {
	//	ZThread::Guard<ZThread::Mutex> z(currentFrameMutex);
		//ZThread::LockedScope lock(&currentFrameMutex);
		//currentFrameMutex.acquire();
		return currentFrame_;
		//currentFrameMutex.release();
	}

	void setCurrentFrame(DirectshowVideoFrame* frame) {
		currentFrame_ = frame;
	}
    CComPtr<ISampleGrabber> pGrabber;
     CComPtr<IBaseFilter>    pSource;
     CComPtr<IBaseFilter>    pASF;
     CComPtr<IGraphBuilder>  pGraph;
     CComPtr<IVideoWindow>   pVideoWindow;
     CComQIPtr<IMediaControl, & IID_IMediaControl> pControl;
     CComQIPtr<IMediaEvent, & IID_IMediaEvent> pEvent;
     CComQIPtr<IMediaSeeking, & IID_IMediaSeeking> pSeeking;
    
     CComQIPtr<IVideoWindow, & IID_IVideoWindow> pWindow;
     CComQIPtr<IBaseFilter, & IID_IBaseFilter> pGrabberBase;
     CComQIPtr<IFileSourceFilter, & IID_IFileSourceFilter> pLoad;
     CComPtr<IPin> pSourcePin;
     CComPtr<IPin> pGrabPin;
	 CComQIPtr<IObjectWithSite> pObjectWithSite;
     CSampleGrabberCB CB;
	 NoDirectVobSub graphBuilderCallback;
protected:
	 DirectshowVideoFrame * currentFrame_;
	// ZThread::Mutex currentFrameMutex;
    
};



//
// CallBack класс для SampleGrabbera
//
// this object is a SEMI-COM object, and can only be created statically.



// Fake out any COM ref counting
STDMETHODIMP_(ULONG) CSampleGrabberCB::AddRef() {
    return 2;
}
STDMETHODIMP_(ULONG) CSampleGrabberCB::Release() {
    return 1;
}

// Fake out any COM QI'ing
STDMETHODIMP CSampleGrabberCB::QueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    if ( riid == IID_ISampleGrabberCB || riid == IID_IUnknown )
    {
        *ppv = (void*) static_cast<ISampleGrabberCB*>(this);
        return NOERROR;
    }

    return E_NOINTERFACE;
}

// Не используется
//
STDMETHODIMP CSampleGrabberCB::SampleCB( double SampleTime, IMediaSample* pSample )
{
    return 0;
}

// Callback ф-ия вызываемая SampleGrabber-ом, в другом потоке
//
STDMETHODIMP CSampleGrabberCB::BufferCB( double SampleTime, BYTE* pBuffer, long BufferSize )
{
	//ZThread::Guard<ZThread::Mutex> z(mutex);

    LONGLONG time = LONGLONG(SampleTime * 10000000);
    prev = time;
    TCHAR buf[256];
    wsprintf(buf, TEXT("%02d:%02d:%02d"), int(SampleTime / 3600), (int)(long(SampleTime) / 60) % 60,
             (int)long(long(SampleTime) % 60) /*,long(SampleTime/100)*/);


    DirectshowVideoFrame *frame = new DirectshowVideoFrame(pBuffer, BufferSize, SampleTime, Width, Height);
    directShowPrivate->setCurrentFrame( frame);
    Grab = false;
	condition.signal();
    return 0;
}


void GrabInfo(Utf8String text){
	OutputDebugString(IuCoreUtils::Utf8ToWstring(text).c_str());
	//LOG(ERROR) << text;
}

bool ShouldStop() {
    return false;
}

DirectshowFrameGrabber::DirectshowFrameGrabber(): d_ptr(new DirectshowFrameGrabberPrivate())
{
}

_COM_SMARTPTR_TYPEDEF(ISampleGrabber, __uuidof(ISampleGrabber));

bool DirectshowFrameGrabber::open(const Utf8String& fileName) {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CString fileNameW = IuCoreUtils::Utf8ToWstring(fileName).c_str();

    //USES_CONVERSION;
    bool IsWMV = false;
    bool IsOther = false;

    HRESULT hr = 1;

    IsOther = false;
    bool IsAvi = true;

    d_ptr->pGrabber.CoCreateInstance( CLSID_SampleGrabber );
    if ( !d_ptr->pGrabber )
    {
		LOG(ERROR) << "Could not create CLSID_SampleGrabber instance.";
        return false;
    }
    d_ptr->pGrabberBase = ( d_ptr->pGrabber );

    // Create the file reader
    //
    // CLSID_WMAsfReader
    if (!IsOther)
    {
        d_ptr->pSource.CoCreateInstance( CLSID_AsyncReader  );
        if ( !d_ptr->pSource )
        {
			LOG(ERROR) << "Couldn't create CLSID_AsyncReader instance.";
            return false;
        }
    }
    if (IsWMV)
    {
        d_ptr->pASF.CoCreateInstance( CLSID_WMAsfReader  );
        if ( !d_ptr->pASF )
        {
			LOG(ERROR) << "Couldn't create CLSID_WMAsfReader instance.";
            return false;
        }
    }
    // Create the graph
    //
    d_ptr->pGraph.CoCreateInstance( CLSID_FilterGraph );
    if ( !d_ptr->pGraph )
    {
		LOG(ERROR) << "Couldn't create CLSID_FilterGraph instance.";
        return false;
    }
	d_ptr->pObjectWithSite = d_ptr->pGraph;
	d_ptr->pObjectWithSite->SetSite(&d_ptr->graphBuilderCallback);
    bool Error = false;
    CComPtr<IGraphBuilder>  pGraph2;
    /*bool IsAVI = true;
    if (!IsAVI)
    {
        pGraph2.CoCreateInstance( CLSID_FilterGraph );

        GrabInfo( tr("Поиск необходимых кодеков...") );
        CComQIPtr<IVideoWindow, & IID_IVideoWindow> pWindow2 = pGraph2;
        if (pWindow2)
        {
            hr = pWindow2->put_AutoShow(OAFALSE);
        }

        hr = pGraph2->RenderFile(fileNameW, NULL);
        if ( FAILED( hr ) )
        {
			LOG(ERROR) << "Couldn't find suitable codecs (probably format is not supported)" << GetMessageForHresult(hr);
            GrabInfo( tr("Невозможно подобрать кодеки (формат не поддерживается).") );
            Error = true;
        }
        CComQIPtr<IMediaControl, & IID_IMediaControl> pControl2( pGraph2);
        pControl2->Stop();
        pGraph2->Abort();
    }*/
    // Put them in the graph
    //
    if (IsWMV)
        hr = d_ptr->pGraph->AddFilter( d_ptr->pASF, L"Source" );  // my
    else if (IsOther)
        hr = d_ptr->pGraph->AddSourceFilter( fileNameW, NULL, &d_ptr->pSource);  // my
    else
        hr = d_ptr->pGraph->AddFilter( d_ptr->pSource, L"Source" );

    if ( FAILED( hr ) )
    {
		LOG(ERROR) << "Couldn't load codec"<<GetMessageForHresult(hr);
        return false;
    }


	

    CMediaType GrabType;
    GrabType.SetType( &MEDIATYPE_Video );
    GrabType.SetSubtype( &MEDIASUBTYPE_RGB24 );
    hr = d_ptr->pGrabber->SetMediaType( &GrabType );

	if ( FAILED( hr ) )
	{
		LOG(ERROR) << "Unable to set MEDIASUBTYPE_RGB24" << GetMessageForHresult(hr);
		return false;
	}

    hr = d_ptr->pGraph->AddFilter( d_ptr->pGrabberBase, L"Grabber" );
	
    // Load the source
    //
    if (IsWMV)
    {
        CComQIPtr<IFileSourceFilter, & IID_IFileSourceFilter> pLoad( d_ptr->pASF);
		hr = pLoad->Load( fileNameW, NULL );
    }
    else if (!IsOther)
    {
        d_ptr->pLoad = ( /*pASF*/ d_ptr->pSource);
		if (!Error) {
            GrabInfo( tr("Загрузка файла...") );
		}
        hr = d_ptr->pLoad->Load(fileNameW, NULL );
    }
    if ( FAILED( hr ) )
    {
		LOG(ERROR) << "Failed to load source file '"<< fileName <<"'"<<GetMessageForHresult(hr);
        return false;
    }

    // Tell the grabber to grab 24-bit video. Must do this
    // before connecting it
    //
     //CMediaType GrabType;
         //  GrabType.SetType( &MEDIATYPE_Video );
          // GrabType.SetSubtype( &MEDIASUBTYPE_RGB24 );
    // GrabType.SetSubtype( &MEDIASUBTYPE_AYUV );*/

   //     hr = pGrabber->SetMediaType( &GrabType );

  

    // Get the output pin and the input pin
    //

    //	CSampleGrabberCB CB;
    if (IsWMV)
       d_ptr-> pSourcePin = DirectShowUtil::GetOutPin( d_ptr->pASF, 1 );
    else
        d_ptr->pSourcePin = DirectShowUtil::GetOutPin( d_ptr->pSource, 0 );
    d_ptr->pGrabPin   = DirectShowUtil::GetInPin( d_ptr->pGrabberBase, 0 );

    // ... and connect them
    //
    if (!Error)
        GrabInfo( tr("Подключение кодеков...") );
    else
        GrabInfo( tr("Ещё одна попытка подключения кодеков...") );
	
	

    hr = d_ptr->pGraph->Connect( d_ptr->pSourcePin, d_ptr->pGrabPin );

    if ( FAILED( hr ) )
    {
		LOG(ERROR) << "Cannot connect filters (format probably is not supported)"<<GetMessageForHresult(hr);
        GrabInfo(  tr("Ошибка соединения фильтров (формат не поддерживается).") );
        return false;
    }

	


	AM_MEDIA_TYPE mt2;
	ZeroMemory(&mt2, sizeof(mt2));
	hr = d_ptr->pGrabber->GetConnectedMediaType( &mt2 );
	if ( FAILED( hr ) )
	{
		LOG(ERROR) << "GetConnectedMediaType failed"<<GetMessageForHresult(hr);
		//GrabInfo(  tr("Ошибка соединения фильтров (формат не поддерживается).") );
		return false;
	}

    // This semi-COM object will receive sample callbacks for us
    //
    //d_ptr->CB.vg = this;

    //d_ptr->CB.SavingThread = &SavingThread;
    d_ptr->CB.BufferEvent = CreateEvent(0, FALSE, FALSE, 0);

    // Ask for the connection media type so we know its size
    //
    AM_MEDIA_TYPE mt;
    hr = d_ptr->pGrabber->GetConnectedMediaType( &mt );
    if (FAILED( hr ))
    {
        //GrabInfo(  TEXT("Unable to determine what we connected.") );
        return false;
    }
    VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*) mt.pbFormat;
    if (!FAILED( hr ))
    {
        d_ptr->CB.Width  = vih->bmiHeader.biWidth;
        d_ptr->CB.Height = vih->bmiHeader.biHeight;
        FreeMediaType( mt );
    }
	//remove DirectVobSub (actually not removing )
	//DirectVobSubUtil::RemoveFromGraph( d_ptr->pGraph);

    // Render the grabber output pin (to a video renderer)
    //
	CComPtr <IPin> pGrabOutPin = DirectShowUtil::GetOutPin( d_ptr->pGrabberBase, 0 );
    // GrabInfo( _T("Trying to render graph.") );
    hr = d_ptr->pGraph->Render( pGrabOutPin );
    if ( FAILED( hr ) )
    {
		LOG(ERROR) << "Error while receiving data from filter's output pins."<<GetMessageForHresult(hr);
        return false;
    }


	//DirectShowUtil::SaveGraphFile(d_ptr->pGraph, _T("Test.grf"));

    // Don't buffer the samples as they pass through
    //
    hr = d_ptr->pGrabber->SetBufferSamples( FALSE );

    // Only grab one at a time, stop stream after
    // grabbing one sample
    //
    hr = d_ptr->pGrabber->SetOneShot( TRUE );

    // Set the callback, so we can grab the one sample
    //
    hr = d_ptr->pGrabber->SetCallback( &d_ptr->CB, 1 );
    //SavingThread.vg = this;
    //SavingThread.Start();
    // Get the seeking interface, so we can seek to a location
    //
    d_ptr->pSeeking = d_ptr->pGraph;

    // Query the graph for the IVideoWindow interface and use it to
    // disable AutoShow.  This will prevent the ActiveMovie window from
    // being displayed while we grab bitmaps from the running movie.
    d_ptr->pWindow = d_ptr->pGraph;
    if (d_ptr->pWindow)
    {
        hr = d_ptr->pWindow->put_AutoShow(OAFALSE);
    }

    // Find a limited number of frames
    LONGLONG duration;
    d_ptr->pSeeking->GetDuration(&duration);
    if (duration == 0)
    {
		LOG(ERROR) << "Cannot determine stream's length.";
        return 0;
    }
	

    duration_ = duration;

    int NumOfFrames = 1;
    d_ptr->CB.directShowPrivate = d_ptr;
    d_ptr->pControl = d_ptr->pGraph;
    d_ptr->pEvent = d_ptr->pGraph;

    long EvCode = 0;

    long EventCode = 0, Param1 = 0, Param2 = 0;
	d_ptr->CB.mutex.acquire();


    return true;
}

bool DirectshowFrameGrabber::seek(int64_t time) {
	//Sleep(300);
    HRESULT  hr;
    int step = 0;
    long EvCode = 0;
    int i = 0;
	d_ptr->setCurrentFrame(0);
	
    //for ( int i = 0; i < NumOfFrames; i++ )
    {
        if (ShouldStop())
        {
            //CanceledByUser = true;
           // break;
        }
        // set position

        REFERENCE_TIME Start = /*(i + 1) * step - step*/ time /*/ 5 * 3*/; // **/(duration/NUM_FRAMES_TO_GRAB);//** UNITS*40*/;
        hr = d_ptr->pGrabber->SetOneShot( TRUE );
        hr = d_ptr->pSeeking->SetPositions( &Start, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 0, AM_SEEKING_NoPositioning);
		if(FAILED(hr)) {
			LOG(WARNING) << "pSeeking->SetPositions failed." << GetMessageForHresult(hr);
		}
        d_ptr->CB.Grab = true;
        hr = d_ptr->pControl->Run( );
		if(FAILED(hr)) {
			LOG(WARNING) << "pControl->Run() failed." << GetMessageForHresult(hr);
		}
        hr = d_ptr->pEvent->WaitForCompletion( INFINITE, &EvCode );
		if(FAILED(hr)) {
			LOG(WARNING) << "pEvent->WaitForCompletion() failed." << GetMessageForHresult(hr);
		}
        d_ptr->pControl->Stop();
		if(FAILED(hr)) {
			LOG(WARNING) << "pControl->Stop() failed." << GetMessageForHresult(hr);
		}
		if ( !d_ptr->currentFrame() ) {
			//bool res = d_ptr->CB.condition.wait(1000);
		}
    }

    return true;
}

AbstractVideoFrame* DirectshowFrameGrabber::grabCurrentFrame() {
    return d_ptr->currentFrame();
}

int64_t DirectshowFrameGrabber::duration() {
    return duration_;
}


void DirectshowFrameGrabber::abort() {

}

 DirectshowFrameGrabber::~DirectshowFrameGrabber() {
	 if ( d_ptr->pControl ) {
		d_ptr->pControl->Stop();
	 }
	 d_ptr->CB.mutex.release();
	 delete d_ptr;
	 ::CoUninitialize();
 }
