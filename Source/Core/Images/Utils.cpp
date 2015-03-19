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
#include <Core/Logging.h>
#include <Func/WinUtils.h>
#include <3rdpart/QColorQuantizer.h>
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

void PrintRichEdit(HWND hwnd, Gdiplus::Graphics* graphics, Gdiplus::Bitmap* background, Gdiplus::Rect layoutArea) {
	using namespace Gdiplus;
	//Calculate the area to render.
	HDC hdc1 = ::GetDC(hwnd);

	double anInchX = 1440 / GetDeviceCaps(hdc1, LOGPIXELSX);
	double anInchY = 1440 / GetDeviceCaps(hdc1, LOGPIXELSY);
			ReleaseDC(hwnd,hdc1);

	//double anInch = 1440.0  /  GetDeviceCaps(hdc1, LOGPIXELSX);

	RECT rectLayoutArea;
	rectLayoutArea.top = (int)(layoutArea.GetTop() * anInchY);
	rectLayoutArea.bottom = (int)(layoutArea.GetBottom() * anInchY);
	rectLayoutArea.left = (int)(layoutArea.GetLeft() *anInchX  );
	rectLayoutArea.right = (int)(layoutArea.GetRight() * anInchX);

	HDC hdc = graphics->GetHDC();
	Gdiplus::Graphics gr2(hdc);
	SolidBrush br(Color(255,255,255));

	// We need to draw background on new HDC, otherwise the text will look ugly
	Status st = gr2.DrawImage(background,layoutArea.GetLeft(),layoutArea.GetTop(),layoutArea.GetLeft(), layoutArea.GetTop(), layoutArea.Width, layoutArea.Height,Gdiplus::UnitPixel/*gr2.GetPageUnit()*/);

	FORMATRANGE fmtRange;
	fmtRange.chrg.cpMax = -1;                    //Indicate character from to character to 
	fmtRange.chrg.cpMin = 0;
	fmtRange.hdc = hdc;                                //Use the same DC for measuring and rendering
	fmtRange.hdcTarget = hdc;                    //Point at printer hDC
	fmtRange.rc = rectLayoutArea;            //Indicate the area on page to print
	fmtRange.rcPage = rectLayoutArea;    //Indicate size of page


	int characterCount = ::SendMessage(hwnd, EM_FORMATRANGE, 1, (LPARAM)&fmtRange);
	LOG(INFO) << "rectLayoutArea"<< rectLayoutArea.left << " "<< rectLayoutArea.top << " "<< rectLayoutArea.right << " "<< rectLayoutArea.bottom << " ";
	LOG(INFO) << "characterCount" << characterCount;

	//Release the device context handle obtained by a previous call
	graphics->ReleaseHDC(hdc);
}

void DrawRoundedRectangle(Gdiplus::Graphics* gr, Gdiplus::Rect r, int d, Gdiplus::Pen* p, Gdiplus::Brush*br){
	using namespace Gdiplus;
	GraphicsPath gp;

	gp.AddArc(r.X, r.Y, d, d, 180, 90);
	gp.AddArc(r.X + r.Width - d, r.Y, d, d, 270, 90);
	gp.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0, 90);
	gp.AddArc(r.X, r.Y + r.Height - d, d, d, 90, 90);
	gp.AddLine(r.X, r.Y + r.Height - d, r.X, r.Y + d/2);
	gp.CloseFigure();
	if ( br ) {
		gr->FillPath(br, &gp);
	}
	gr->DrawPath(p, &gp);

}


bool SaveImage(Image* img, const CString& filename, SaveImageFormat format, int Quality)
{
	if (format == sifDetectByExtension ) {
		CString ext = WinUtils::GetFileExt(filename);
		ext.MakeLower();
		if ( ext == L"jpg" || ext == L"jpeg") {
			format = sifJPEG;
		} else if ( ext == L"gif" ) {
			format = sifGIF;
		} else  {
			format = sifPNG;
		}
	}


	std::auto_ptr<Bitmap> quantizedImage;
	TCHAR szImgTypes[3][4] = {_T("jpg"), _T("png"), _T("gif")};
	TCHAR szMimeTypes[3][12] = {_T("image/jpeg"), _T("image/png"), _T("image/gif")};

	
//	IU_CreateFilePath(filename);

	CLSID clsidEncoder;
	EncoderParameters eps;
	eps.Count = 1;

	if (format == sifJPEG) // JPEG
	{
		eps.Parameter[0].Guid = EncoderQuality;
		eps.Parameter[0].Type = EncoderParameterValueTypeLong;
		eps.Parameter[0].NumberOfValues = 1;
		eps.Parameter[0].Value = &Quality;
	}
	else if (format == sifPNG)      // PNG
	{
		eps.Parameter[0].Guid = EncoderCompression;
		eps.Parameter[0].Type = EncoderParameterValueTypeLong;
		eps.Parameter[0].NumberOfValues = 1;
		eps.Parameter[0].Value = &Quality;
	} else if (format == sifGIF) {
		QColorQuantizer quantizer;
		quantizedImage.reset ( quantizer.GetQuantized(img, QColorQuantizer::Octree, (Quality < 50) ? 16 : 256) );
		if (quantizedImage.get() != 0) {
			img = quantizedImage.get();
		}
	}

	Gdiplus::Status result;

	if (GetEncoderClsid(szMimeTypes[format], &clsidEncoder) != -1) {
		if (format == sifJPEG) {
			result = img->Save(filename, &clsidEncoder, &eps);
		} else {
			result = img->Save(filename, &clsidEncoder);
		}
	} else {
		return false;
	}
	
	if (result != Ok) {
		LOG(ERROR) <<  _T("Could not save image at path \r\n") + filename;
		return false;
	}

	return true;
}
