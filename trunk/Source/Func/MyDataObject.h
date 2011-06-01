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
#ifndef MYDATAOBJECT_H
#define MYDATAOBJECT_H


#pragma once
#include "atlheaders.h"

class CMyDataObject : public IDataObject
{
	public:
		CMyDataObject();
		~CMyDataObject();

		void Reset();
		void AddFile(LPCTSTR FileName);

		// IUnknown members.
		 HRESULT __stdcall QueryInterface(REFIID iid,void **ppvObject);
		 ULONG __stdcall AddRef();
		 ULONG __stdcall Release();
	        
		 // IDataObject members.
		 HRESULT __stdcall GetData(FORMATETC *pFormatEtc,STGMEDIUM *pmedium);
		 HRESULT __stdcall GetDataHere(FORMATETC *pFormatEtc,STGMEDIUM *pmedium);
		 HRESULT __stdcall QueryGetData(FORMATETC *pFormatEtc);
		 HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC *pFormatEct,FORMATETC *pFormatEtcOut);
		 HRESULT __stdcall SetData(FORMATETC *pFormatEtc,STGMEDIUM *pMedium,BOOL fRelease);
		 HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
		 HRESULT __stdcall DAdvise(FORMATETC *pFormatEtc,DWORD advf,IAdviseSink *,DWORD *);
		 HRESULT __stdcall DUnadvise(DWORD dwConnection);
		 HRESULT __stdcall EnumDAdvise(IEnumSTATDATA **ppEnumAdvise);
	private:
		long m_lRefCount;
		FORMATETC m_FormatEtc;
		STGMEDIUM m_StgMedium;
		int TotalLength;
		CAtlArray<CString> m_FileItems;
		bool IsFormatSupported(FORMATETC *pFormatEtc);
};

#endif // MYDATAOBJECT_H