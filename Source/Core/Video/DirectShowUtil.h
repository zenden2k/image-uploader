#ifndef IU_CORE_VIDEO_DIRECTSHOWUTIL_H
#define IU_CORE_VIDEO_DIRECTSHOWUTIL_H

#pragma once
#include "atlheaders.h"
#define __AFX_H__ // little hack for avoiding __POSITION type redefinition
#include <objbase.h>
#include <streams.h>
#undef __AFX_H__
#include <qedit.h>
#include <comdef.h>
#include <comip.h>
#include <atlbase.h>

namespace DirectShowUtil {
    // Вспомогательные функции
    // для нахождения входных и выходных пинов DirectShow фильтров
    HRESULT GetPin(IBaseFilter* pFilter, PIN_DIRECTION dirrequired,  int iNum, IPin** ppPin);
    IPin*  GetInPin ( IBaseFilter* pFilter, int Num );
    IPin*  GetOutPin( IBaseFilter* pFilter, int Num );
    GUID GuidFromString(const CString& guidStr);
    CComPtr<IBaseFilter> FindFilterByClassID(CComPtr<IGraphBuilder> graphBuilder, GUID classID);
    HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);
}

#endif