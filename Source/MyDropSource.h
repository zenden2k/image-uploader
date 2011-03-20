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

#pragma once

class CMyDropSource : public IDropSource
{
private:
	long m_lRefCount;

public:
	CMyDropSource();
	~CMyDropSource();

	// IUnknown members.
    HRESULT __stdcall QueryInterface(REFIID iid,void ** ppvObject);
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();

	// IDropSource members.
    HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed,DWORD grfKeyState);
    HRESULT __stdcall GiveFeedback(DWORD dwEffect);
};
