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
#include "../atlheaders.h"
#include "MyDropSource.h"

CMyDropSource::CMyDropSource()
{
	m_lRefCount = 1;
}

CMyDropSource::~CMyDropSource()
{
}

HRESULT __stdcall CMyDropSource::QueryInterface(REFIID iid,void **ppvObject)
{
    // Check to see what interface has been requested.
    if (iid == IID_IDropSource || iid == IID_IUnknown)
    {
        AddRef();
        *ppvObject = this;
        return S_OK;
    }
    else
    {
        *ppvObject = 0;
        return E_NOINTERFACE;
    }
}

ULONG __stdcall CMyDropSource::AddRef()
{
    // Increment object reference count.
    return InterlockedIncrement(&m_lRefCount);
}

ULONG __stdcall CMyDropSource::Release()
{
    // Decrement object reference count.
	LONG lCount = InterlockedDecrement(&m_lRefCount);
		
	if (lCount == 0)
	{
		delete this;
		return 0;
	}
	else
	{
		return lCount;
	}
}

HRESULT __stdcall CMyDropSource::QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState)
{
	// If the escape key has been pressed we cancel the operation.
    if (fEscapePressed == TRUE)
        return DRAGDROP_S_CANCEL;

	// If the left button has been released we should drop.
    if ((grfKeyState & MK_LBUTTON) == 0)
        return DRAGDROP_S_DROP;

    return S_OK;
}

HRESULT __stdcall CMyDropSource::GiveFeedback(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}
