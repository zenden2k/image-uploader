/*
Image Uploader - program for uploading images/files to Internet
Copyright (C) 2007-2015 ZendeN <zenden2k@gmail.com>

HomePage:    http://zenden.ws/imageuploader

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Utils.h"

#include <gdiplus.h>
#include <Core/Logging.h>
#include <Func/WinUtils.h>
#include <3rdpart/QColorQuantizer.h>
#include <math.h>
#include <stdint.h>
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
	//LOG(INFO) << "rectLayoutArea"<< rectLayoutArea.left << " "<< rectLayoutArea.top << " "<< rectLayoutArea.right << " "<< rectLayoutArea.bottom << " ";
	//LOG(INFO) << "characterCount" << characterCount;

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

Gdiplus::Bitmap* IconToBitmap(HICON ico)
{
	ICONINFO ii; 
	GetIconInfo(ico, &ii);
	BITMAP bmp; 
	GetObject(ii.hbmColor, sizeof(bmp), &bmp);

	Gdiplus::Bitmap temp(ii.hbmColor, NULL);
	Gdiplus::BitmapData lockedBitmapData;
	memset(&lockedBitmapData, 0, sizeof(lockedBitmapData));
	Gdiplus::Rect rc(0, 0, temp.GetWidth(), temp.GetHeight());

	Gdiplus::Status st = temp.LockBits(&rc, Gdiplus::ImageLockModeRead, temp.GetPixelFormat(), &lockedBitmapData);


	Gdiplus::Bitmap* image = new Gdiplus::Bitmap(
		lockedBitmapData.Width, lockedBitmapData.Height, lockedBitmapData.Stride,
		PixelFormat32bppARGB, reinterpret_cast<BYTE *>(lockedBitmapData.Scan0));

	temp.UnlockBits(&lockedBitmapData);
	return image;
}

class DummyBitmap {
public:
	DummyBitmap(uint8_t* data,  int stride, int width, int height, int channel=0) {
		data_ = data;
		stride_ = stride;
		width_ = width;
		channel_ = channel;
		dataSize_ = stride * height;
		height_ = height;
	}
	inline uint8_t& operator[](int i) {
		int pos = (i/width_)*stride_ + (i%width_)*4 + channel_;
		if ( pos >= dataSize_) {
			return data_[0];
		} else {
			return data_[pos];
		}
	}
protected:
	uint8_t* data_;
	int stride_;
	int channel_;
	int width_;
	int dataSize_;
	int height_;
};

#if !defined(_MSC_VER) || _MSC_VER < 1800 

float round(float number)
{
	return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

#endif

int* boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
{
	float wIdeal = sqrt((12*sigma*sigma/n)+1);  // Ideal averaging filter width 
	int wl = floor(wIdeal);  if(wl%2==0) wl--;
	int wu = wl+2;

	float mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
	float m = round(mIdeal);


	int* sizes = new int[n];
	for(int i=0; i<n; i++) sizes[i]=i<m?wl:wu;
	return sizes;
}

void boxBlurH_4 (DummyBitmap& scl, DummyBitmap& tcl, int w, int h, int r) {
	float iarr = 1.0 / (r+r+1);
	for(int i=0; i<h; i++) {
		int ti = i*w, li = ti, ri = ti+r;
		int fv = scl[ti], lv = scl[ti+w-1], val = (r+1)*fv;
		for(int j=0; j<r; j++) val += scl[ti+j];
		for(int j=0  ; j<=r ; j++) { 
			val += scl[ri++] - fv       ;   
			tcl[ti++] = round(val*iarr); }
		for(int j=r+1; j<w-r; j++) { val += scl[ri++] - scl[li++];   tcl[ti++] = round(val*iarr); }
		for(int j=w-r; j<w  ; j++) { val += lv        - scl[li++];   tcl[ti++] = round(val*iarr); }
	}
}
void boxBlurT_4 (DummyBitmap&  scl, DummyBitmap&  tcl, int w, int h, int r) {
	float iarr = 1.0 / (r+r+1);
	for(int i=0; i<w; i++) {
		int ti = i, li = ti, ri = ti+r*w;
		int fv = scl[ti], lv = scl[ti+w*(h-1)], val = (r+1)*fv;
		for(int j=0; j<r; j++) val += scl[ti+j*w];
		for(int j=0  ; j<=r ; j++) { 
			val += scl[ri] - fv     ;  
			tcl[ti] = round(val*iarr);  
			ri+=w; 
			ti+=w; }
		for(int j=r+1; j<h-r; j++) { val += scl[ri] - scl[li];  tcl[ti] = round(val*iarr);  li+=w; ri+=w; ti+=w; }
		for(int j=h-r; j<h  ; j++) { val += lv      - scl[li];  tcl[ti] = round(val*iarr);  li+=w; ti+=w; }
	}
}

void boxBlur_4 (DummyBitmap& scl, DummyBitmap& tcl, int w, int h, int r) {
	//for(int i=0; i<scl.length; i++) tcl[i] = scl[i];
	boxBlurH_4(tcl, scl, w, h, r);
	boxBlurT_4(scl, tcl, w, h, r);
}


void gaussBlur_4 (DummyBitmap& scl, DummyBitmap& tcl, int w, int h, int r) {
	int* bxs = boxesForGauss(r, 3);
	boxBlur_4 (scl, tcl, w, h, (bxs[0]-1)/2);
	boxBlur_4 (tcl, scl, w, h, (bxs[1]-1)/2);
	boxBlur_4 (scl, tcl, w, h, (bxs[2]-1)/2);
	delete bxs;
}
uint8_t *prevBuf = 0;
int prevSize=0;

void BlurCleanup() {
	delete[] prevBuf;
	prevBuf = 0;
}

void ApplyGaussianBlur(Gdiplus::Bitmap* bm, int x,int y, int w, int h, int radius) {
	using namespace Gdiplus;
	Rect rc(x, y, w, h);

	BitmapData dataSource;


	if (bm->LockBits(& rc, ImageLockModeRead|ImageLockModeWrite, PixelFormat32bppARGB, & dataSource) == Ok)
	{
		uint8_t * source= (uint8_t *) dataSource.Scan0;
		UINT stride;
		if (dataSource.Stride > 0) { stride = dataSource.Stride;
		} else {
			stride = - dataSource.Stride;
		}
		uint8_t *buf;
		if ( prevBuf && prevSize >= stride * h ) {
			buf = prevBuf;
		} else {
			delete[] prevBuf;
			buf = new uint8_t[stride * h];
			prevSize = stride * h;
			prevBuf = buf;
		}
		
		memcpy(buf, source,stride * h);

		//bm->UnlockBits(&dataSource);

		DummyBitmap srcR(source,  stride, w, h, 0);
		DummyBitmap dstR(buf,  stride, w,h, 0);
		DummyBitmap srcG(source,  stride, w, h, 1);
		DummyBitmap dstG(buf,  stride, w,h, 1);
		DummyBitmap srcB(source,  stride,  w, h,2);
		DummyBitmap dstB(buf,  stride, w, h, 2);
		/*DummyBitmap srcB(source,  stride,  w, h,3);
		DummyBitmap dstB(buf,  stride, w, h, 3);*/
		gaussBlur_4(srcR, dstR, w, h, radius);
		gaussBlur_4(srcG, dstG, w, h, radius);
		gaussBlur_4(srcB, dstB, w, h, radius);
		/*buf2[rand() % stride * h]=0;*/
		//memset(buf2, 255, stride * h/2);
		/*-if (bm->LockBits(& rc, ImageLockModeWrite, PixelFormat24bppRGB, & dataSource) == Ok)
		{
			memcpy(pRowSource ,  buf2,stride * h);
			bm->UnlockBits(&dataSource);
		}*/
		//gaussBlur_4(srcR, dstR, w, h, 10);
		//memcpy(pRowSource ,  buf,stride * h);
		memcpy(source ,  buf,stride * h);
		bm->UnlockBits(&dataSource);
		//delete[] buf;
	//	delete[] buf2;
	}

}

Gdiplus::Bitmap* LoadImageFromFileWithoutLocking(const WCHAR* fileName) {
	using namespace Gdiplus;
	Bitmap src( fileName );
	if ( src.GetLastStatus() != Ok ) {
		return 0;
	}
	Bitmap *dst = new Bitmap(src.GetWidth(), src.GetHeight(), PixelFormat32bppARGB);

	BitmapData srcData;
	BitmapData dstData;
	Rect rc(0, 0, src.GetWidth(), src.GetHeight());

	if (src.LockBits(& rc, ImageLockModeRead, PixelFormat32bppARGB, & srcData) == Ok)
	{
		if ( dst->LockBits(& rc, ImageLockModeWrite, PixelFormat32bppARGB, & dstData) == Ok ) {
			uint8_t * srcBits = (uint8_t *) srcData.Scan0;
			uint8_t * dstBits = (uint8_t *) dstData.Scan0;
			unsigned int stride;
			if (srcData.Stride > 0) { 
				stride = srcData.Stride;
			} else {
				stride = - srcData.Stride;
			}
			memcpy(dstBits, srcBits, src.GetHeight() * stride);

			dst->UnlockBits(&dstData);
		}
		src.UnlockBits(&srcData);
	}
	return dst;
}
