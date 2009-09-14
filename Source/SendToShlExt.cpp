/*
    Image Uploader - application for uploading images/files to Internet
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

#include "stdafx.h"
#include "SendToShlExt.h"

// CSendToShlExt
// SendToShlExt.cpp : Implementation of CSendToShlExt

HRESULT CSendToShlExt::DragEnter ( IDataObject* pDataObj, DWORD grfKeyState,
											 POINTL pt, DWORD* pdwEffect )
{
	TCHAR          szItem [MAX_PATH];
	UINT           uNumFiles;
	HGLOBAL        hglobal;
	HDROP          hdrop;
	*buffer = 0;

	FORMATETC tc;
	tc.cfFormat = CF_HDROP;
	tc.ptd = 0;
	tc.dwAspect = 1;
	tc.lindex = DVASPECT_CONTENT;
	tc.tymed =  TYMED_HGLOBAL;

	if(pDataObj->QueryGetData(&tc)!=S_OK ) 
	{
		*pdwEffect=DROPEFFECT_NONE;
		return 0;
	}

	STGMEDIUM ddd;
	pDataObj->GetData(&tc, &ddd);

	// handle just hGlobal types
	switch (ddd.tymed)
	{
	case TYMED_MFPICT:
	case TYMED_HGLOBAL:
		break;
	default:
		return 0;
		// default -- falls through to error condition...
	}
	hglobal = ddd.hGlobal;

	if (NULL != hglobal)
	{
		hdrop = (HDROP) GlobalLock (hglobal);

		uNumFiles = DragQueryFile (hdrop, 0xFFFFFFFF, NULL, 0); //retrieving count of files

		for(UINT uFile = 0;uFile <uNumFiles; uFile++)
		{
			if (0 != DragQueryFile( hdrop, uFile, szItem, MAX_PATH))
			{
				CmdLine.AddParam(szItem);
			}
		}
		DragFinish(hdrop);
		GlobalUnlock ( hglobal );
	}

	*pdwEffect = DROPEFFECT_COPY;
	return S_OK;
}

HRESULT CSendToShlExt::Drop ( IDataObject* pDataObj, DWORD grfKeyState,
                              POINTL pt, DWORD* pdwEffect )
{
	PostQuitMessage(0);
    *pdwEffect = DROPEFFECT_COPY;
    return S_OK;
}

