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
    // ��������������� �������
    // ��� ���������� ������� � �������� ����� DirectShow ��������
    HRESULT GetPin(IBaseFilter* pFilter, PIN_DIRECTION dirrequired,  int iNum, IPin** ppPin);
    CComPtr<IPin>  GetInPin ( IBaseFilter* pFilter, int Num );
    CComPtr<IPin>  GetOutPin( IBaseFilter* pFilter, int Num );
    GUID GuidFromString(const CString& guidStr);
    CComPtr<IBaseFilter> FindFilterByClassID(CComPtr<IGraphBuilder> graphBuilder, GUID classID);
    HRESULT SaveGraphFile(IGraphBuilder *pGraph, WCHAR *wszPath);
}

#endif