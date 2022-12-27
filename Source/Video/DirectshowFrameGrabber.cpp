/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include <mutex>
#include "atlheaders.h"
#include "Core/Utils/CoreUtils.h"
#include <windows.h>
#define __AFX_H__ // little hack for avoiding __POSITION type redefinition
#include <objbase.h>
#include <streams.h>
#undef __AFX_H__
#include <qedit.h>
#include <comdef.h>
#include <comip.h>
#include <Strmif.h>

#include "AbstractVideoFrame.h"
#include "Core/Logging.h"
#include "Core/3rdpart/dxerr.h"
#include "DirectShowUtil.h"
#include "Core/i18n/Translator.h"
#include "NoDirectVobSub.h"
#include "DirectshowVideoFrame.h"

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

class CSampleGrabberCB : public ISampleGrabberCB
{
public:
    CSampleGrabberCB() {
		directShowPrivate = nullptr;
		Width = 0;
		Height = 0;
		Grab = true;
		BufferEvent = nullptr;
		prev = 0;
    }
    std::mutex mutex_;
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
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // Fake out any COM QI'ing
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP SampleCB(double SampleTime, IMediaSample* pSample) override;

    STDMETHODIMP BufferCB(double SampleTime, BYTE* pBuffer, long BufferSize) override;
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
        return currentFrame_;
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
     VIDEOINFOHEADER vih;
protected:
     DirectshowVideoFrame * currentFrame_; 
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
    LONGLONG time = LONGLONG(SampleTime * 10000000);
    prev = time;

    DirectshowVideoFrame *frame = new DirectshowVideoFrame(pBuffer, BufferSize, static_cast<int64_t>(SampleTime), Width, Height);
    directShowPrivate->setCurrentFrame( frame);
    Grab = false;
    return 0;
}


void GrabInfo(std::string text){
    OutputDebugString(IuCoreUtils::Utf8ToWstring(text).c_str());
    //LOG(ERROR) << text;
}

DirectshowFrameGrabber::DirectshowFrameGrabber() : 
    duration_(0),
    d_ptr(new DirectshowFrameGrabberPrivate())

{
}

_COM_SMARTPTR_TYPEDEF(ISampleGrabber, __uuidof(ISampleGrabber));

bool DirectshowFrameGrabber::open(const std::string& fileName) {
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

        GrabInfo( t_r("Поиск необходимых кодеков...") );
        CComQIPtr<IVideoWindow, & IID_IVideoWindow> pWindow2 = pGraph2;
        if (pWindow2)
        {
            hr = pWindow2->put_AutoShow(OAFALSE);
        }

        hr = pGraph2->RenderFile(fileNameW, NULL);
        if ( FAILED( hr ) )
        {
            LOG(ERROR) << "Couldn't find suitable codecs (probably format is not supported)" << GetMessageForHresult(hr);
            GrabInfo( t_r("Невозможно подобрать кодеки (формат не поддерживается).") );
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
    if (IsWMV)
    {
        CComQIPtr<IFileSourceFilter, & IID_IFileSourceFilter> pLoad( d_ptr->pASF);
        hr = pLoad->Load( fileNameW, NULL );
    }
    else if (!IsOther)
    {
        d_ptr->pLoad = ( /*pASF*/ d_ptr->pSource);
        if (!Error) {
            GrabInfo( tr("Loading file...") );
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

    //    CSampleGrabberCB CB;
    if (IsWMV)
       d_ptr-> pSourcePin = DirectShowUtil::GetOutPin( d_ptr->pASF, 1 );
    else
        d_ptr->pSourcePin = DirectShowUtil::GetOutPin( d_ptr->pSource, 0 );
    d_ptr->pGrabPin   = DirectShowUtil::GetInPin( d_ptr->pGrabberBase, 0 );

    // ... and connect them
    //
    if (!Error)
        GrabInfo( tr("Connecting codecs") );
    else
        GrabInfo( tr("Trying again to connect filters...") );
    
    hr = d_ptr->pGraph->Connect( d_ptr->pSourcePin, d_ptr->pGrabPin );

    if ( FAILED( hr ) )
    {
        LOG(ERROR) << "Cannot connect filters (format probably is not supported)"<<GetMessageForHresult(hr);
        GrabInfo(  tr("Cannot connect filters (format probably is not supported).") );
        return false;
    }

    AM_MEDIA_TYPE mt2;
    ZeroMemory(&mt2, sizeof(mt2));
    hr = d_ptr->pGrabber->GetConnectedMediaType( &mt2 );
    if ( FAILED( hr ) )
    {
        LOG(ERROR) << "GetConnectedMediaType failed"<<GetMessageForHresult(hr);
        return false;
    }

    // This semi-COM object will receive sample callbacks for us
    d_ptr->CB.BufferEvent = CreateEvent(0, FALSE, FALSE, 0);

    // Ask for the connection media type so we know its size
    AM_MEDIA_TYPE mt;
    hr = d_ptr->pGrabber->GetConnectedMediaType( &mt );
    if (FAILED( hr ))
    {
        //GrabInfo(  TEXT("Unable to determine what we connected.") );
        return false;
    }
    if (!FAILED(hr))
    {
        if ((mt.formattype == FORMAT_VideoInfo) &&
            (mt.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
            (mt.pbFormat != NULL))
        {
            VIDEOINFOHEADER* vih = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);

            d_ptr->vih = *vih;
            d_ptr->CB.Width = vih->bmiHeader.biWidth;
            d_ptr->CB.Height = vih->bmiHeader.biHeight;
            FreeMediaType(mt);
        } else {
            LOG(ERROR) << "Invalid mediatype format";
            return false;
        }
    }

    // Render the grabber output pin (to a video renderer)
    //
    CComPtr <IPin> pGrabOutPin = DirectShowUtil::GetOutPin( d_ptr->pGrabberBase, 0 );
    // GrabInfo( _T("Trying to render graph.") );
    //DirectShowUtil::SaveGraphFile(d_ptr->pGraph, _T("d:\\Test.grf"));
    hr = d_ptr->pGraph->Render( pGrabOutPin );
    if ( FAILED( hr ) )
    {
        LOG(ERROR) << "Error while receiving data from filter's output pins."<<GetMessageForHresult(hr);
        return false;
    }

    

    // Don't buffer the samples as they pass through
    hr = d_ptr->pGrabber->SetBufferSamples( FALSE );

    // Only grab one at a time, stop stream after
    // grabbing one sample
    hr = d_ptr->pGrabber->SetOneShot( TRUE );

    // Set the callback, so we can grab the one sample
    hr = d_ptr->pGrabber->SetCallback( &d_ptr->CB, 1 );
    //SavingThread.vg = this;
    //SavingThread.Start();
    // Get the seeking interface, so we can seek to a location
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
    d_ptr->CB.directShowPrivate = d_ptr.get();
    d_ptr->pControl = d_ptr->pGraph;
    d_ptr->pEvent = d_ptr->pGraph;

    //d_ptr->CB.mutex_.lock();
    return true;
}

bool DirectshowFrameGrabber::seek(int64_t time) {
    HRESULT  hr;
    int step = 0;
    long EvCode = 0;
    int i = 0;
    d_ptr->setCurrentFrame(0);
    
    //for ( int i = 0; i < NumOfFrames; i++ )
    {
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

     ::CoUninitialize();
 }
