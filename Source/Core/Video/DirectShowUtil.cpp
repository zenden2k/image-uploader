#include "DirectShowUtil.h"
#include "atlheaders.h"
#include "Core/Logging.h"

namespace DirectShowUtil {;

HRESULT GetPin( IBaseFilter* pFilter, PIN_DIRECTION dirrequired, int iNum, IPin** ppPin)
{
	IEnumPins* pEnum;
	//CComPtr<IEnumPins> pEnum;
	*ppPin = NULL;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
		return hr;

	ULONG ulFound;
	IPin* pPin;
	hr = E_FAIL;

	while (S_OK == pEnum->Next(1, &pPin, &ulFound))
	{
		PIN_DIRECTION pindir = (PIN_DIRECTION)3;

		pPin->QueryDirection(&pindir);
		if (pindir == dirrequired)
		{
			if (iNum == 0)
			{
				*ppPin = pPin;      // Return the pin's interface
				hr = S_OK;          // Found requested pin, so clear error
				break;
			}
			iNum--;
		}

		pPin->Release();
	}

	return hr;
}

IPin* GetInPin( IBaseFilter* pFilter, int nPin )
{
	IPin* pComPin = 0;
	//CComPtr<IPin> pComPin = 0;
	GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
	return pComPin;
}

IPin* GetOutPin( IBaseFilter* pFilter, int nPin )
{
	IPin* pComPin = 0;
	//CComPtr<IPin> pComPin = 0;
	GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
	return pComPin;
}

GUID GuidFromString(const CString& guidStr)
{
	GUID res = GUID_NULL;
	HRESULT hr = ::CLSIDFromString( guidStr, &res );
	return res;
}

CComPtr<IBaseFilter> FindFilterByClassID(CComPtr<IGraphBuilder> graphBuilder, GUID classID)
{
	CComPtr<IBaseFilter> filterFound;

	CComPtr<IEnumFilters> ienumFilt;
		HRESULT hr = graphBuilder->EnumFilters(&ienumFilt);
		if (SUCCEEDED(hr) && ienumFilt )
		{ 
			ULONG iFetched;
			IBaseFilter* filter;
		
			do
			{
				hr = ienumFilt->Next(1, &filter, &iFetched);
				if (SUCCEEDED(hr) && iFetched == 1) {
					GUID filterGuid;
					HRESULT hr2 = filter->GetClassID(&filterGuid);

					if (filterGuid == classID)
					{
						filterFound.Attach(filter);
						return filterFound;
					}
					filter->Release();
					filter = 0;
				}
			} while (iFetched == 1 && SUCCEEDED(hr));
		}

	return filterFound;
} 

HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath) 
{
	const WCHAR wszStreamName[] = L"ActiveMovieGraph"; 
	HRESULT hr;

	IStorage *pStorage = NULL;
	hr = StgCreateDocfile(
		wszPath,
		STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
		0, &pStorage);
	if(FAILED(hr)) 
	{
		return hr;
	}

	IStream *pStream;
	hr = pStorage->CreateStream(
		wszStreamName,
		STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
		0, 0, &pStream);
	if (FAILED(hr)) 
	{
		pStorage->Release();    
		return hr;
	}

	IPersistStream *pPersist = NULL;
	pGraph->QueryInterface(IID_IPersistStream, (void**)&pPersist);
	hr = pPersist->Save(pStream, TRUE);
	pStream->Release();
	pPersist->Release();
	if (SUCCEEDED(hr)) 
	{
		hr = pStorage->Commit(STGC_DEFAULT);
	}
	pStorage->Release();
	return hr;
}



}