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

#include "Utils.h"

#include <gdiplus.h>
using namespace Gdiplus;


int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;           // number of image encoders
	UINT size = 0;          // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if ( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}



Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
	using namespace Gdiplus;
	HRSRC hrsrc = FindResource(hInstance, szResName, szResType);
	if (!hrsrc)
		return 0;
	// "Fake" HGLOBAL - look at MSDN
	HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
	DWORD sz = SizeofResource(hInstance, hrsrc);
	void* ptr1 = LockResource(hg1);
	HGLOBAL hg2 = GlobalAlloc(GMEM_FIXED, sz);

	// Copy raster data
	CopyMemory(LPVOID(hg2), ptr1, sz);
	IStream* pStream;

	// TRUE means free memory at Release
	HRESULT hr = CreateStreamOnHGlobal(hg2, TRUE, &pStream);
	if (FAILED(hr))
		return 0;

	// use load from IStream
	Gdiplus::Bitmap* image = Bitmap::FromStream(pStream);
	pStream->Release();
	// GlobalFree(hg2);
	return image;
}
