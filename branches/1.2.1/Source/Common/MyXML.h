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

#pragma once

#include "MarkupMSXML.h"

//using namespace MSXML2;

class CMyXml: public  CMarkupMSXML
{
	public:
	 bool GetAttrib(LPCTSTR szAttrib, int &Value)
	 {
		 CString Res =CMarkupMSXML::GetAttrib(szAttrib);
		 if(Res.IsEmpty()) return false;
		 Value = _ttoi(Res);
		 return true;
	 }
	 bool GetAttrib(LPCTSTR szAttrib, bool &Value)
	 {
		 CString Res = CMarkupMSXML::GetAttrib(szAttrib);
		 if(Res.IsEmpty()) return false;
		 Value = (bool) _ttoi(Res);
		 return true;
	 }
	  bool GetAttrib(LPCTSTR szAttrib, CString &Value)
	 {
		 Value = CMarkupMSXML::GetAttrib(szAttrib);
		 return Value.IsEmpty();
	 }

	  bool GetData( int &Value)
	 {
		 CString Res =CMarkupMSXML::GetData();
		 if(Res.IsEmpty()) return false;
		 Value = _ttoi(Res);
		 return true;
	 }
	 bool GetData( bool &Value)
	 {
		 CString Res = CMarkupMSXML::GetData();
		 if(Res.IsEmpty()) return false;
		 Value = (bool) _ttoi(Res);
		 return true;
	 }
	bool GetData(CString &Value)
	{
		 Value = CMarkupMSXML::GetData();
		 return Value.IsEmpty();
	}

	bool FormatDOMDocument (MSXML2::IXMLDOMDocument *pDoc, IStream *pStream)
	{using namespace MSXML2;
		// Thanks to Google
		CComPtr <MSXML2::IMXWriter> pMXWriter; // Create the writer
		if (FAILED (pMXWriter.CoCreateInstance(__uuidof (MSXML2::MXXMLWriter40), NULL, CLSCTX_ALL)))
		 {
			  return false;
		 }
		 CComPtr <MSXML2::ISAXContentHandler> pISAXContentHandler;
		 if (FAILED (pMXWriter.QueryInterface(&pISAXContentHandler)))
		 {
			  return false;
		 }
		 CComPtr <MSXML2::ISAXErrorHandler> pISAXErrorHandler;
		 if (FAILED (pMXWriter.QueryInterface (&pISAXErrorHandler)))
		 {
			  return false;
		 }
		 CComPtr <MSXML2::ISAXDTDHandler> pISAXDTDHandler;
		 if (FAILED (pMXWriter.QueryInterface (&pISAXDTDHandler)))
		 {
			  return false;
		 }

		if (FAILED (pMXWriter ->put_omitXMLDeclaration (VARIANT_FALSE)) ||
			 FAILED (pMXWriter ->put_standalone (VARIANT_TRUE)) ||
			 FAILED (pMXWriter ->put_indent (VARIANT_TRUE)) ||
			 FAILED (pMXWriter ->put_encoding (L"utf-16")))
		{
			 return false;
		}

		CComPtr <MSXML2::ISAXXMLReader> pSAXReader;
		if (FAILED (pSAXReader.CoCreateInstance (__uuidof (SAXXMLReader), NULL, CLSCTX_ALL)))
		{
			return false;
		}

		if (FAILED (pSAXReader ->putContentHandler (pISAXContentHandler)) ||
		 FAILED (pSAXReader ->putDTDHandler (pISAXDTDHandler)) ||
		 FAILED (pSAXReader ->putErrorHandler (pISAXErrorHandler)) ||
		 FAILED (pSAXReader ->putProperty (
		 (unsigned short *)L"http://xml.org/sax/properties/lexical-handler", CComVariant (pMXWriter))) ||
		 FAILED (pSAXReader ->putProperty (
		 (unsigned short *)L"http://xml.org/sax/properties/declaration-handler", CComVariant (pMXWriter))))
		{
			return false;
		}
		LPTSTR sz = pDoc->xml;
		//MessageBox(0,sz,0,0);
		// Perform the write
		return SUCCEEDED (pMXWriter ->put_output (CComVariant (pStream))) &&
				SUCCEEDED (pSAXReader ->parse (CComVariant (sz)));
	}

	bool Save( LPCTSTR szFileName )
	{
		// Own Save function because default function doesn't save xml with indents
		STATSTG StatInfo;
		IStream* pStrmDest = NULL ;
		if (FAILED(SHCreateStreamOnFile(szFileName, STGM_WRITE | STGM_CREATE, &pStrmDest)))
			return false ;
		WORD BOM = 0xFEFF;
		DWORD res;
		pStrmDest->Write(&BOM, sizeof(BOM),&res);
		bool ress = FormatDOMDocument(m_pDOMDoc,pStrmDest);
		
		if(!ress) 
		{
			LPTSTR sz=m_pDOMDoc->xml;
			CString data = sz;
			if(!data.IsEmpty())
			{
				data.Replace(_T(">\<"), _T(">\r\n<"));
				pStrmDest->Write((LPCTSTR)data, data.GetLength()*sizeof(TCHAR),&res);
				pStrmDest->Release();
			}
			else
			{
				pStrmDest->Release();
				CMarkupMSXML::Save(szFileName);
			}
		}
		else pStrmDest->Release();
		return true;
	}
};