/*

    Uptooda - free application for uploading images/files to the Internet

    Copyright 2007-2025 Sergey Svistunov (zenden2k@gmail.com)

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
#ifndef MYDATAOBJECT_H
#define MYDATAOBJECT_H

#pragma once
#include "atlheaders.h"

class CMyDataObject : public IDataObject
{
    public:
        CMyDataObject();
        virtual ~CMyDataObject();

        void Reset();
        void AddFile(LPCTSTR FileName);

        // IUnknown members.
         HRESULT __stdcall QueryInterface(REFIID iid,void **ppvObject) override;
         ULONG __stdcall AddRef() override;
         ULONG __stdcall Release() override;

         // IDataObject members.
         HRESULT __stdcall GetData(FORMATETC *pFormatEtc,STGMEDIUM *pStgMedium) override;
         HRESULT __stdcall GetDataHere(FORMATETC *pFormatEtc,STGMEDIUM *pmedium) override;
         HRESULT __stdcall QueryGetData(FORMATETC *pFormatEtc) override;
         HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatEct,FORMATETC *pFormatEtcOut) override;
         HRESULT __stdcall SetData(FORMATETC *pFormatEtc,STGMEDIUM *pMedium,BOOL fRelease) override;
         HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc) override;
         HRESULT __stdcall DAdvise(FORMATETC *pFormatEtc,DWORD advf,IAdviseSink *,DWORD *) override;
         HRESULT __stdcall DUnadvise(DWORD dwConnection) override;
         HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppEnumAdvise) override;
    private:
        long m_lRefCount;
        FORMATETC m_FormatEtc;
        STGMEDIUM m_StgMedium;
        int TotalLength;
        CAtlArray<CString> m_FileItems;
        bool IsFormatSupported(FORMATETC *pFormatEtc);
};

#endif // MYDATAOBJECT_H
