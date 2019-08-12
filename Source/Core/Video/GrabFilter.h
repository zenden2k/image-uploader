#ifndef IU_CORE_VIDEO_DUMPFILTER_H
#define IU_CORE_VIDEO_DUMPFILTER_H

#include <windows.h>
#include "streams.h"

class CGrabInputPin;
class CGrabFilter;

class CGrabCallback {
public:
    virtual ~CGrabCallback() = default;
    virtual void OnDumpSample(REFERENCE_TIME SampleTime, BYTE* pBuffer, long BufferSize, BITMAPINFOHEADER* bih) = 0;
};

class CGrabFilter : public CBaseFilter
{
    CGrabInputPin* m_inputPin;
    CCritSec m_Lock;
    CCritSec m_ReceiveLock;
    bool m_grab;
    CGrabCallback* m_callback;
    CMediaType m_mediaType;
    CPosPassThru *m_pPosition;
    REFERENCE_TIME m_segmentStart;
public:
    CGrabFilter(LPUNKNOWN pUnk);

    CBasePin * GetPin(int n) override;
    int GetPinCount() override;

    // called from pin
    HRESULT Receive(IMediaSample* pSample);

    // called from pin
    HRESULT EndOfStream();

    // called from pin 
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    HRESULT SetMediaType(const CMediaType *pmt);
    STDMETHODIMP NonDelegatingQueryInterface(const IID& riid, void** ppv) override;

    void SetGrab(bool grab);
    void SetCallback(CGrabCallback* cb);
};

//  Pin object

class CGrabInputPin : public CRenderedInputPin
{
    CCritSec* m_pReceiveLock;
    CGrabFilter *m_pDumpFilter;
public:

    CGrabInputPin(CGrabFilter *pFilter, CCritSec *pLock, CCritSec *pReceiveLock, HRESULT *phr);

    // Do something with this media sample
    STDMETHODIMP Receive(IMediaSample *pSample) override;
    STDMETHODIMP EndOfStream(void) override;
    STDMETHODIMP ReceiveCanBlock() override;

    // Check if the pin can support this specific proposed type and format
    HRESULT CheckMediaType(const CMediaType *) override;

    HRESULT SetMediaType(const CMediaType *pmt) override;

    // Track NewSegment
    STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate) override;
};

#endif