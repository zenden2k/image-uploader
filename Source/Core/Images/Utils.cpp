/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

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

#include "Utils.h"

#include <stdint.h>
#include <cmath>
#include <cassert>

#include <boost/format.hpp>
#include <libbase64.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include "3rdpart/GdiplusH.h"
#include "Core/Logging.h"
#include "Func/WinUtils.h"
#include "3rdpart/QColorQuantizer.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Gui/Dialogs/LogWindow.h"
#include "Core/ServiceLocator.h"
#include "Func/Library.h"
#include "Core/AppParams.h"

namespace ImageUtils {


using namespace Gdiplus;

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;           // number of image encoders
    UINT size = 0;          // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;

    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(malloc(size));
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

std::unique_ptr<Gdiplus::Bitmap> BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
    using namespace Gdiplus;
    HRSRC hrsrc = FindResource(hInstance, szResName, szResType);
    if (!hrsrc)
        return nullptr;
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
        return nullptr;

    // use load from IStream
    std::unique_ptr<Gdiplus::Bitmap> image(Bitmap::FromStream(pStream));
    pStream->Release();
    // GlobalFree(hg2);
    return image;
}

void PrintRichEdit(HWND hwnd, Gdiplus::Graphics* graphics, Gdiplus::Bitmap* background, Gdiplus::Rect layoutArea) {
    using namespace Gdiplus;
    //Calculate the area to render.
    HDC hdc1 = ::GetDC(hwnd);

    double anInchX = 1440.0 / GetDeviceCaps(hdc1, LOGPIXELSX);
    double anInchY = 1440.0 / GetDeviceCaps(hdc1, LOGPIXELSY);
            ReleaseDC(hwnd,hdc1);

    //double anInch = 1440.0  /  GetDeviceCaps(hdc1, LOGPIXELSX);

    RECT rectLayoutArea;
    rectLayoutArea.top = static_cast<int>(layoutArea.GetTop() * anInchY);
    rectLayoutArea.bottom = static_cast<int>(layoutArea.GetBottom() * anInchY);
    rectLayoutArea.left = static_cast<int>(layoutArea.GetLeft() *anInchX  );
    rectLayoutArea.right = static_cast<int>(layoutArea.GetRight() * anInchX);

    HDC hdc = graphics->GetHDC();
    Gdiplus::Graphics gr2(hdc);
    SolidBrush br(Color(255,255,255));

    // We need to draw background on new HDC, otherwise the text will look ugly
    gr2.DrawImage(background,layoutArea.GetLeft(),layoutArea.GetTop(),layoutArea.GetLeft(), layoutArea.GetTop(), layoutArea.Width, layoutArea.Height,Gdiplus::UnitPixel/*gr2.GetPageUnit()*/);

    FORMATRANGE fmtRange;
    fmtRange.chrg.cpMax = -1;                    //Indicate character from to character to 
    fmtRange.chrg.cpMin = 0;
    fmtRange.hdc = hdc;                                //Use the same DC for measuring and rendering
    fmtRange.hdcTarget = hdc;                    //Point at printer hDC
    fmtRange.rc = rectLayoutArea;            //Indicate the area on page to print
    fmtRange.rcPage = rectLayoutArea;    //Indicate size of page

    /*int characterCount = */::SendMessage(hwnd, EM_FORMATRANGE, 1, reinterpret_cast<LPARAM>(&fmtRange));

    //Release the device context handle obtained by a previous call
    graphics->ReleaseHDC(hdc);
}

void DrawRoundedRectangle(Gdiplus::Graphics* gr, Gdiplus::Rect r, int d, Gdiplus::Pen* p, Gdiplus::Brush*br){
    using namespace Gdiplus;
    GraphicsPath gp;
//    d = min(min(d, r.Width),r.Height);
    gp.AddArc(r.X, r.Y, d, d, 180, 90);
    gp.AddArc(max(r.X + r.Width - d,r.X), r.Y, d, d, 270, 90);
    gp.AddArc(max(r.X, r.X + r.Width - d), max(r.Y, r.Y + r.Height - d), d, d, 0, 90);
    gp.AddArc(r.X, max(r.Y, r.Y + r.Height - d), d, d, 90, 90);
    //gp.AddLine(r.X, max(r.Y, r.Y + r.Height - d), r.X, min(r.Y + d/2, r.GetBottom()));+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
    //TCHAR szImgTypes[3][4] = {_T("jpg"), _T("png"), _T("gif")};
    TCHAR szMimeTypes[3][12] = {_T("image/jpeg"), _T("image/png"), _T("image/gif")};

    
//    IU_CreateFilePath(filename);

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
        if (result == Gdiplus::Win32Error) {
            LOG(ERROR) << _T("Could not save image at path \r\n") + filename << "\r\n" << WinUtils::GetLastErrorAsString();
        } else {
            LOG(ERROR) << _T("Could not save image at path \r\n") + filename << "\r\nGdiPlus error:" << GdiplusStatusToString(result);
        }
        return false;
    }
    uint64_t fileSize = IuCoreUtils::getFileSize(W2U(filename));
    if (!fileSize) {
        LOG(ERROR) << _T("Could not save image at path \r\n") + filename + "\r\nOutput file's size is zero.";
        return false;
    }
    return true;
}

std::unique_ptr<Gdiplus::Bitmap> IconToBitmap(HICON ico)
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

    if (st != Gdiplus::Ok) {
        return nullptr;
    }

    std::unique_ptr<Gdiplus::Bitmap> image(new Gdiplus::Bitmap(
        lockedBitmapData.Width, lockedBitmapData.Height, lockedBitmapData.Stride,
        PixelFormat32bppARGB, reinterpret_cast<BYTE *>(lockedBitmapData.Scan0)));

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


int* boxesForGauss(float sigma, int n)  // standard deviation, number of boxes
{
    float wIdeal = sqrt((12*sigma*sigma/n)+1);  // Ideal averaging filter width 
    int wl = static_cast<int>(floor(wIdeal));  
    if(wl%2==0) wl--;
    int wu = wl+2;

    float mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n)/(-4*wl - 4);
    float m = round(mIdeal);


    int* sizes = new int[n];
    for(int i=0; i<n; i++) sizes[i]=i<m?wl:wu;
    return sizes;
}

void boxBlurH_4 (DummyBitmap& scl, DummyBitmap& tcl, int w, int h, int r) {
    float iarr = static_cast<float>(1.0 / (r + r + 1));
    for(int i=0; i<h; i++) {
        int ti = i*w, li = ti, ri = ti+r;
        int fv = scl[ti], lv = scl[ti+w-1], val = (r+1)*fv;
        for(int j=0; j<r; j++) val += scl[ti+j];
        for(int j=0  ; j<=r ; j++) { 
            val += scl[ri++] - fv;   
            tcl[ti++] = static_cast<uint8_t>(round(val*iarr)); 
        }
        for(int j=r+1; j<w-r; j++) {
            val += scl[ri++] - scl[li++];   
            tcl[ti++] = static_cast<uint8_t>(round(val*iarr));
        }
        for(int j=w-r; j<w  ; j++) {
            val += lv        - scl[li++];   
            tcl[ti++] = static_cast<uint8_t>(round(val*iarr));
        }
    }
}
void boxBlurT_4 (DummyBitmap&  scl, DummyBitmap&  tcl, int w, int h, int r) {
    float iarr = static_cast<float>(1.0 / (r+r+1));
    for(int i=0; i<w; i++) {
        int ti = i, li = ti, ri = ti+r*w;
        int fv = scl[ti], lv = scl[ti+w*(h-1)], val = (r+1)*fv;
        for(int j=0; j<r; j++) val += scl[ti+j*w];
        for(int j=0  ; j<=r ; j++) { 
            val += scl[ri] - fv     ;  
            tcl[ti] = static_cast<uint8_t>(round(val*iarr));
            ri+=w; 
            ti+=w; }
        for(int j=r+1; j<h-r; j++) {
            val += scl[ri] - scl[li];  
            tcl[ti] = static_cast<uint8_t>(round(val*iarr));
            li+=w; ri+=w; ti+=w;
        }
        for(int j=h-r; j<h  ; j++) {
            val += lv      - scl[li];  
            tcl[ti] = static_cast<uint8_t>(round(val*iarr));
            li+=w; 
            ti+=w;
        }
    }
}

void boxBlur_4 (DummyBitmap& scl, DummyBitmap& tcl, int w, int h, int r) {
    //for(int i=0; i<scl.length; i++) tcl[i] = scl[i];
    boxBlurH_4(tcl, scl, w, h, r);
    boxBlurT_4(scl, tcl, w, h, r);
}

void gaussBlur_4 (DummyBitmap& scl, DummyBitmap& tcl, int w, int h, int r) {
    int* bxs = boxesForGauss(static_cast<float>(r), 3);
    boxBlur_4 (scl, tcl, w, h, (bxs[0]-1)/2);
    boxBlur_4 (tcl, scl, w, h, (bxs[1]-1)/2);
    boxBlur_4 (scl, tcl, w, h, (bxs[2]-1)/2);
    delete[] bxs;
}
uint8_t *prevBuf = 0;
unsigned int prevSize=0;

void BlurCleanup() {
    delete[] prevBuf;
    prevBuf = 0;
    prevSize = 0;
}

void ApplyGaussianBlur(Gdiplus::Bitmap* bm, int x,int y, int w, int h, int radius) {
    using namespace Gdiplus;
    Rect rc(x, y, w, h);

    BitmapData dataSource;

    if (bm->LockBits(& rc, ImageLockModeRead|ImageLockModeWrite, PixelFormat32bppARGB, & dataSource) == Ok)
    {
        uint8_t * source = reinterpret_cast<uint8_t *>(dataSource.Scan0);
        assert(static_cast<UINT>(h) == dataSource.Height);
        UINT stride;
        if (dataSource.Stride > 0) { stride = dataSource.Stride;
        } else {
            stride = - dataSource.Stride;
        }
        uint8_t *buf;
        size_t myStride = 4 * (w /*& ~3*/);
        size_t bufSize = myStride * h;

        if (prevBuf && prevSize >= bufSize) {
            buf = prevBuf;
        } else {
            delete[] prevBuf;
            
            buf = new uint8_t[bufSize];
            prevSize = bufSize;
            prevBuf = buf;
        }
        
        uint8_t *bufCur = buf;
        uint8_t *sourceCur = source;
       
        for (int i = 0; i < h; i++) {
            memcpy(bufCur, sourceCur, myStride);
            bufCur += myStride;
            sourceCur += stride;
        }

        //memcpy(buf, source, stride * (h - 1) + w * 4 /*PixelFormat32bppARGB*/);

        DummyBitmap srcR(source,  stride, w, h, 0);
        DummyBitmap dstR(buf, myStride, w, h, 0);
        DummyBitmap srcG(source,  stride, w, h, 1);
        DummyBitmap dstG(buf, myStride, w, h, 1);
        DummyBitmap srcB(source,  stride,  w, h,2);
        DummyBitmap dstB(buf, myStride, w, h, 2);

        gaussBlur_4(srcR, dstR, w, h, radius);
        gaussBlur_4(srcG, dstG, w, h, radius);
        gaussBlur_4(srcB, dstB, w, h, radius);

        bufCur = buf;
        sourceCur = source;
        for (int i = 0; i < h; i++) {
            memcpy(sourceCur, bufCur, myStride);
            bufCur += myStride;
            sourceCur += stride;
        }

        //memcpy(source, buf, stride * (h-1)+w * 4 /*PixelFormat32bppARGB*/);
        bm->UnlockBits(&dataSource);
    }

}

std::unique_ptr<Gdiplus::Bitmap> LoadImageFromFileWithoutLocking(const WCHAR* fileName) {
    using namespace Gdiplus;

    std::unique_ptr<Bitmap> src(LoadImageFromFileExtended(fileName));
    if ( !src || src->GetLastStatus() != Ok ) {
        return nullptr;
    }
    std::unique_ptr<Gdiplus::Bitmap> dst = std::make_unique<Bitmap>(src->GetWidth(), src->GetHeight(), PixelFormat32bppARGB);

    BitmapData srcData;
    BitmapData dstData;
    Rect rc(0, 0, src->GetWidth(), src->GetHeight());

    if (src->LockBits(& rc, ImageLockModeRead, PixelFormat32bppARGB, & srcData) == Ok)
    {
        if ( dst->LockBits(& rc, ImageLockModeWrite, PixelFormat32bppARGB, & dstData) == Ok ) {
            uint8_t * srcBits = reinterpret_cast<uint8_t *>(srcData.Scan0);
            uint8_t * dstBits = reinterpret_cast<uint8_t *>(dstData.Scan0);
            unsigned int stride;
            if (srcData.Stride > 0) { 
                stride = srcData.Stride;
            } else {
                stride = - srcData.Stride;
            }
            memcpy(dstBits, srcBits, src->GetHeight() * stride);

            dst->UnlockBits(&dstData);
        }
        src->UnlockBits(&srcData);
    }
    return dst;
}

Gdiplus::Color StringToColor(const std::string& str) {
    if ( str.empty() ) {
        return Gdiplus::Color();
    }
    if ( str[0] == '#' ) {
        std::string res = IuStringUtils::Replace(str, "#", "0x");
        return strtoul(res.c_str(), 0, 0);
    } else if ( str.substr(0,4) == "rgba" && str.length() >= 14 ) {
        std::vector<std::string> tokens;
        IuStringUtils::Split(str.substr(5, str.length()-5 ),",", tokens,4);
        if ( tokens.size() == 4 ) {
            return Gdiplus::Color(static_cast<BYTE>(round(atof(tokens[3].c_str())*255)), static_cast<BYTE>(atoi(tokens[0].c_str())),
                static_cast<BYTE>(atoi(tokens[1].c_str())), static_cast<BYTE>(atoi(tokens[2].c_str())));
        }
    } else if ( str.substr(0,4) == "rgb" && str.length() >= 13 ) {
        std::vector<std::string> tokens;
        IuStringUtils::Split(str.substr(4, str.length()-4 ),",", tokens,3);
        if ( tokens.size() == 3 ) {
            return Gdiplus::Color( static_cast<BYTE>(atoi(tokens[0].c_str())), static_cast<BYTE>(atoi(tokens[1].c_str())), static_cast<BYTE>(atoi(tokens[2].c_str())));
        }
    }
    return Gdiplus::Color();
}



struct BGRA_COLOR
{
    BYTE b;
    BYTE g;
    BYTE r;
    BYTE a;
};

// hack for stupid GDIplus
void Gdip_RemoveAlpha(Gdiplus::Bitmap& source, Gdiplus::Color color )
{
    using namespace Gdiplus;
    Rect r( 0, 0, source.GetWidth(),source.GetHeight() );
    BitmapData  bdSrc;
    source.LockBits( &r,  ImageLockModeRead , PixelFormat32bppARGB,&bdSrc);

    BYTE* bpSrc = reinterpret_cast<BYTE*>(bdSrc.Scan0);

    //bpSrc += (int)sourceChannel;

    for ( int i = r.Height * r.Width; i > 0; i-- )
    {
        BGRA_COLOR * c = reinterpret_cast<BGRA_COLOR *>(bpSrc);

        if(c->a!=255)
        {
            //c = 255;
            DWORD * d= reinterpret_cast<DWORD*>(bpSrc);
            *d= color.ToCOLORREF();
            c ->a= 255;
        }
        bpSrc += 4;
    }
    source.UnlockBits( &bdSrc );
}

bool CopyBitmapToClipboard(HWND hwnd, HDC dc, Gdiplus::Bitmap* bm, bool preserveAlpha)
{
    if ( OpenClipboard(hwnd) ){
        EmptyClipboard();
        if ( !preserveAlpha )
            Gdip_RemoveAlpha(*bm,Color(255,255,255,255));
        HBITMAP out=0;
        bm->GetHBITMAP(Color(255,255,255,255),&out);
        CDC origDC,  destDC;
        origDC.CreateCompatibleDC(dc);
        CBitmap destBmp;
        destBmp.CreateCompatibleBitmap(dc, bm->GetWidth(), bm->GetHeight());
        HBITMAP oldOrigBmp = origDC.SelectBitmap(out);
        destDC.CreateCompatibleDC(dc);
        HBITMAP oldDestBmp = destDC.SelectBitmap(destBmp);
        destDC.BitBlt(0,0,bm->GetWidth(),bm->GetHeight(),origDC,0,0,SRCCOPY);
        destDC.SelectBitmap(oldDestBmp);
        origDC.SelectBitmap(oldOrigBmp);
        SetClipboardData(CF_BITMAP, destBmp);
        CloseClipboard(); 
        DeleteObject(out);
        return true;
    } else {
        LOG(ERROR) << ("Cannot copy image to clipboard.") << "\r\n" << WinUtils::GetLastErrorAsString();
    }
    return false;
}

void DrawGradient(Graphics& gr, Rect rect, Color& Color1, Color& Color2)
{
    Bitmap bm(rect.Width, rect.Height, &gr);
    Graphics temp(&bm);
    LinearGradientBrush
        brush(/*TextBounds*/ Rect(0, 0, rect.Width, rect.Height), Color1, Color2, LinearGradientModeVertical);

    temp.FillRectangle(&brush, Rect(0, 0, rect.Width, rect.Height));
    gr.DrawImage(&bm, rect.X, rect.Y);
}

void DrawStrokedText(Graphics& gr, LPCTSTR Text, RectF Bounds, const Font& font, const Color& ColorText, const Color& ColorStroke,
    int HorPos, int VertPos,
    int width)
{
    RectF OriginalTextRect, NewTextRect;
    FontFamily ff;
    font.GetFamily(&ff);
    gr.SetPageUnit(UnitPixel);
    gr.MeasureString(Text, -1, &font, PointF(0, 0), &OriginalTextRect);

    Font NewFont(&ff, 48, font.GetStyle(), UnitPixel);
    gr.MeasureString(Text, -1, &NewFont, RectF(0, 0, 5000, 1600), &NewTextRect);
    OriginalTextRect.Height = OriginalTextRect.Height - OriginalTextRect.Y;
    float newwidth, newheight;
    newheight = OriginalTextRect.Height;
    newwidth = OriginalTextRect.Height / NewTextRect.Height * NewTextRect.Width;
    float k = 2 * width * NewTextRect.Height / OriginalTextRect.Height;
    SolidBrush br(ColorText);
    Bitmap temp(static_cast<int>(NewTextRect.Width), static_cast<int>(NewTextRect.Height), &gr);

    Graphics gr_temp(&temp);
    StringFormat format;
    gr_temp.SetPageUnit(UnitPixel);
    GraphicsPath path;
    gr_temp.SetSmoothingMode(SmoothingModeHighQuality);
    path.AddString(Text, -1, &ff, static_cast<int>(NewFont.GetStyle()), NewFont.GetSize(), Point(0, 0), &format);

    Pen pen(ColorStroke, static_cast<float>(k));
    pen.SetAlignment(PenAlignmentCenter);

    float x, y;
    gr_temp.DrawPath(&pen, &path);
    gr_temp.FillPath(&br, &path);
    gr.SetSmoothingMode(SmoothingModeHighQuality);
    gr.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    if (HorPos == 0)
        x = 2;
    else
        if (HorPos == 1)
            x = (Bounds.Width - newwidth) / 2;
        else
            x = (Bounds.Width - newwidth) - 2;

    if (VertPos == 0)
        y = 2;
    else
        if (VertPos == 1)
            y = (Bounds.Height - newheight) / 2;
        else
            y = (Bounds.Height - newheight) - 2;

    gr.DrawImage(&temp, static_cast<int>(Bounds.GetLeft() + x), static_cast<int>(Bounds.GetTop() + y), static_cast<int>(newwidth), static_cast<int>(newheight));
}

// hack for stupid GDIplus
void changeAplhaChannel(Bitmap& source, Bitmap& dest, int sourceChannel, int destChannel)
{
    Rect r(0, 0, source.GetWidth(), source.GetHeight());
    BitmapData bdSrc;
    BitmapData bdDst;
    source.LockBits(&r, ImageLockModeRead, PixelFormat32bppARGB, &bdSrc);
    dest.LockBits(&r, ImageLockModeWrite, PixelFormat32bppARGB, &bdDst);

    BYTE* bpSrc = reinterpret_cast<BYTE*>(bdSrc.Scan0);
    BYTE* bpDst = reinterpret_cast<BYTE*>(bdDst.Scan0);
    bpSrc += static_cast<int>(sourceChannel);
    bpDst += static_cast<int>(destChannel);

    for (int i = r.Height * r.Width; i > 0; i--)
    {
        // if(*bpSrc != 255)
        {
            *bpDst = static_cast<BYTE>((float(255 - *bpSrc) / 255) *  *bpDst);
        }

        /*if(*bpDst == 0)
        {
        bpDst -=(int)destChannel;
        *bpDst = 0;
        *(bpDst+1) = 0;
        *(bpDst+2) = 0;
        bpDst +=(int)destChannel;
        }*/
        bpSrc += 4;
        bpDst += 4;
    }
    source.UnlockBits(&bdSrc);
    dest.UnlockBits(&bdDst);
}

Rect MeasureDisplayString(Graphics& graphics, CString text, RectF boundingRect, Font& font) {
    CharacterRange charRange(0, text.GetLength());
    Region pCharRangeRegions;
    StringFormat strFormat;
    strFormat.SetMeasurableCharacterRanges(1, &charRange);
    graphics.MeasureCharacterRanges(text, text.GetLength(), &font, boundingRect, &strFormat, 1, &pCharRangeRegions);
    Rect rc;
    pCharRangeRegions.GetBounds(&rc, &graphics);

    return rc;
}

bool MySaveImage(Image* img, const CString& szFilename, CString& szBuffer, int Format, int Quality, LPCTSTR Folder)
{
    if (Format == -1)
        Format = 0;
    
    TCHAR szImgTypes[3][4] = { _T("jpg"), _T("png"), _T("gif") };
    
    CString szNameBuffer;
    TCHAR szBuffer2[MAX_PATH];
    if (IuCommonFunctions::IsImage(szFilename))
        szNameBuffer = WinUtils::GetOnlyFileName(szFilename);
    else
        szNameBuffer = szFilename;
    CString userFolder;
    if (Folder)
        userFolder = Folder;
    if (userFolder.Right(1) != _T('\\'))
        userFolder += _T('\\');
    wsprintf(szBuffer2, _T(
        "%s%s.%s"), static_cast<LPCTSTR>(Folder ? userFolder : AppParams::instance()->tempDirectoryW()), static_cast<LPCTSTR>(szNameBuffer),
        /*(int)GetTickCount(),*/ szImgTypes[Format]);
    CString resultFilename = WinUtils::GetUniqFileName(szBuffer2);
    WinUtils::CreateFilePath(resultFilename);
    bool res = SaveImageToFile(img, resultFilename, nullptr, Format, Quality);
    
    szBuffer = resultFilename;
    return res;
}

CString GdiplusStatusToString(Gdiplus::Status statusID) {
    switch (statusID) {

    case Gdiplus::Ok: return "Ok"; break;
    case Gdiplus::GenericError: return "GenericError"; break;
    case Gdiplus::InvalidParameter: return "InvalidParameter"; break;
    case Gdiplus::OutOfMemory: return "OutOfMemory"; break;
    case Gdiplus::ObjectBusy: return "ObjectBusy"; break;
    case Gdiplus::InsufficientBuffer: return "InsufficientBuffer"; break;
    case Gdiplus::NotImplemented: return "NotImplemented"; break;
    case Gdiplus::Win32Error: return "Win32Error"; break;
    case Gdiplus::Aborted: return "Aborted"; break;
    case Gdiplus::FileNotFound: return "FileNotFound"; break;
    case Gdiplus::ValueOverflow: return "ValueOverflow"; break;
    case Gdiplus::AccessDenied: return "AccessDenied"; break;
    case Gdiplus::UnknownImageFormat: return "UnknownImageFormat"; break;
    case Gdiplus::FontFamilyNotFound: return "FontFamilyNotFound"; break;
    case Gdiplus::FontStyleNotFound: return "FontStyleNotFound"; break;
    case Gdiplus::NotTrueTypeFont: return "NotTrueTypeFont"; break;
    case Gdiplus::UnsupportedGdiplusVersion: return "UnsupportedGdiplusVersion"; break;
    case Gdiplus::GdiplusNotInitialized: return "GdiplusNotInitialized"; break;
    case Gdiplus::PropertyNotFound: return "PropertyNotFound"; break;
    case Gdiplus::PropertyNotSupported: return "PropertyNotSupported"; break;
    //case Gdiplus::ProfileNotFound: return "ProfileNotFound"; break;
    default: return "Status Type Not Found."; break;

    };
}
bool SaveImageToFile(Gdiplus::Image* img, const CString& fileName, IStream* stream, int Format, int Quality, CString* mimeType) {
    std::unique_ptr<Bitmap> quantizedImage;
    
    Gdiplus::Status result;
    TCHAR szMimeTypes[3][12] = { _T("image/jpeg"), _T("image/png"), _T("image/gif") };
    CLSID clsidEncoder;
    EncoderParameters eps;
    eps.Count = 1;

    if (Format == 0) // JPEG
    {
        eps.Parameter[0].Guid = EncoderQuality;
        eps.Parameter[0].Type = EncoderParameterValueTypeLong;
        eps.Parameter[0].NumberOfValues = 1;
        eps.Parameter[0].Value = &Quality;
    } else if (Format == 1) // PNG
    {
        eps.Parameter[0].Guid = EncoderCompression;
        eps.Parameter[0].Type = EncoderParameterValueTypeLong;
        eps.Parameter[0].NumberOfValues = 1;
        eps.Parameter[0].Value = &Quality;
    } else if (Format == 2) {
        QColorQuantizer quantizer;
        quantizedImage.reset(quantizer.GetQuantized(img, QColorQuantizer::Octree, (Quality < 50) ? 16 : 256));
        if (quantizedImage.get() != 0)
            img = quantizedImage.get();
    }
    if (GetEncoderClsid(szMimeTypes[Format], &clsidEncoder) != -1) {
        if (Format == 0)
            result = stream ? img->Save(stream, &clsidEncoder, &eps)  : img->Save(fileName, &clsidEncoder, &eps);
        else
            result = stream ? img->Save(stream, &clsidEncoder, &eps)  : img->Save(fileName, &clsidEncoder);
        if (mimeType) {
            *mimeType = szMimeTypes[Format];
        }
    } else {
        ServiceLocator::instance()->logger()->write(logError, _T("Image Converter"), _T("Could not find suitable converter"));
        return false;
    }
    

    if (result != Ok) {
        int lastError = GetLastError();
        CString error = GdiplusStatusToString(result);
        if (result == Gdiplus::Win32Error) {
            error += L"\r\n" + WinUtils::FormatWindowsErrorMessage(lastError) + L"";
        }
        ServiceLocator::instance()->logger()->write(logError, _T("Image Converter"), _T("Could not save image at path \r\n") + fileName + L"\r\nGdiplus status=" + error);
        return false;
    }

    return true;
}

CRect CenterRect(CRect r1, const CRect& intoR2)
{
    r1.OffsetRect((intoR2.Width() - r1.Width()) / 2, (intoR2.Height() - r1.Height()) / 2);
    return r1;
}

void DrawRect(Bitmap& gr, Color& color, Rect rect)
{
    int i;
    SolidBrush br(color);
    for (i = rect.X; i < rect.Width; i++)
    {
        gr.SetPixel(i, 0, color);
        gr.SetPixel(i, rect.Height - 1, color);
    }

    for (i = rect.Y; i < rect.Height; i++)
    {
        gr.SetPixel(0, i, color);
        gr.SetPixel(rect.Width - 1, i, color);
    }
}
/*int propertyCount = bm->GetPropertyCount();
std::vector<PROPID> props;
props.resize(propertyCount);
bm->GetPropertyIdList(propertyCount, &props[0]);*/

// Allocates storage for entire file 'file_name' and returns contents and size
bool ExUtilReadFile(const wchar_t* const file_name, uint8_t** data, size_t* data_size) {
    int ok;
    uint8_t* file_data;
    size_t file_size;
    FILE* in;

    if (data == nullptr || data_size == nullptr) return 0;
    *data = nullptr;
    *data_size = 0;

    in = _wfopen(file_name, L"rb");
    if (in == nullptr) {
        LOG(ERROR) << "cannot open input file " << file_name;
        return false;
    }
    fseek(in, 0, SEEK_END);
    file_size = ftell(in);
    fseek(in, 0, SEEK_SET);
    try {
        file_data = new uint8_t[file_size];
    } catch (std::exception &) {
        LOG(ERROR) << "Unable to allocate " << file_size << " bytes";
        fclose(in);
        return 0;
    }
    if (file_data == nullptr) return 0;
    ok = (fread(file_data, 1, file_size, in) == file_size);
    fclose(in);

    if (!ok) {
        LOG(ERROR) << boost::format("Could not read %d bytes of data from file ") % file_size << file_name;
        delete[] file_data;
        return false;
    }
    *data = file_data;
    *data_size = file_size;
    return true;
}

struct WebPPic {
    int width;
    int height; 
    uint8_t* rgba;
    bool animated;
};

bool ReadWebP(const uint8_t* const data, size_t data_size, WebPPic* pic) {
    VP8StatusCode status = VP8_STATUS_OK;
    WebPDecoderConfig config;
    WebPDecBuffer* const output_buffer = &config.output;
    WebPBitstreamFeatures* const bitstream = &config.input;

    if (!WebPInitDecoderConfig(&config)) {
        LOG(ERROR) << "Library version mismatch!\n";
        return false;
    }

    status = WebPGetFeatures(data, data_size, bitstream);
    if (status != VP8_STATUS_OK) {
        LOG(WARNING) << "Error loading input webp data, status code=" << status;
        return false;
    }
    pic->animated = !!bitstream->has_animation;

    if (bitstream->has_animation) {
        WebPData webp_data;
        WebPDataInit(&webp_data);
        webp_data.bytes = data;
        webp_data.size = data_size;
        WebPAnimDecoder* dec;
        WebPAnimInfo anim_info;
        WebPAnimDecoderOptions options;
        memset(&options, 0, sizeof(options));
        options.color_mode = MODE_BGRA;

        dec = WebPAnimDecoderNew(&webp_data, &options);
        if (dec == nullptr) {
            LOG(WARNING) << "Error parsing webp image";
            return false;
        }

        if (!WebPAnimDecoderGetInfo(dec, &anim_info)) {
            LOG(WARNING) << "Error getting global info about the animation";
            return false;
        }

        pic->width = anim_info.canvas_width;
        pic->height = anim_info.canvas_height;

        // Decode just first frame
        if (WebPAnimDecoderHasMoreFrames(dec)) {
            uint8_t* frame_rgba;
            int timestamp;

            if (!WebPAnimDecoderGetNext(dec, &frame_rgba, &timestamp)) {
                return false;
            }
            unsigned int frameSize = anim_info.canvas_width*anim_info.canvas_height * 4;
            pic->rgba = new uint8_t[frameSize];
            memcpy(pic->rgba, frame_rgba, frameSize);
        }
        WebPAnimDecoderDelete(dec);

    } else {
        //output_buffer->colorspace = has_alpha ? MODE_RGBA : MODE_RGB;

        pic->rgba = WebPDecodeBGRA(data, data_size, &pic->width, &pic->height);
    }
    if (!pic->rgba) {
        return false;
    }
    /*if (status == VP8_STATUS_OK) {
          /*pic->rgba = output_buffer->u.RGBA.rgba;
          pic->stride = output_buffer->u.RGBA.stride;
          pic->width = output_buffer->width;
          pic->height = output_buffer->height;*/


    /*ok = has_alpha ? WebPPictureImportRGBA(pic, rgba, stride)
              : WebPPictureImportRGB(pic, rgba, stride);*
      }*/


    WebPFreeDecBuffer(output_buffer);
    return true;
}

short GetImageOrientation(Image* img) {
    UINT totalBufferSize = 0, numProperties;
    short orient = 0;
    img->GetPropertySize(&totalBufferSize, &numProperties);

    if (totalBufferSize) {
        PropertyItem* all_items = (PropertyItem*)malloc(totalBufferSize);
        if (!all_items) {
            return orient;
        }
        img->GetAllPropertyItems(totalBufferSize, numProperties, all_items);

        for (UINT j = 0; j < numProperties; ++j) {
            if (all_items[j].id == 0x0112) {
                
                memcpy(&orient, all_items[j].value, sizeof(orient));

                
                break; // only orientation
            }
        }
        free(all_items);
    }
    return orient;
}

bool RotateAccordingToOrientation(short orient, Image* img, bool removeTag) {
    if (orient == 2) {
        img->RotateFlip(RotateNoneFlipX);
    } else if (orient == 3) {
        img->RotateFlip(Rotate180FlipNone);
    } else if (orient == 4) {
        img->RotateFlip(RotateNoneFlipY);
    } else if (orient == 5) {
        img->RotateFlip(Rotate90FlipX);
    } else if (orient == 6) {
        img->RotateFlip(Rotate270FlipXY);
    } else if (orient == 7) {
        img->RotateFlip(Rotate270FlipX);
    } else if (orient == 8) {
        img->RotateFlip(Rotate90FlipXY);
    }
    // This EXIF data is now invalid and should be removed.
    //img->RemovePropertyItem(PropertyTagOrientation);
    return false;
}

std::unique_ptr<Bitmap> LoadImageFromFileExtended(const CString& fileName) {
    std::unique_ptr<Bitmap>  bm;
    CString ext = WinUtils::GetFileExt(fileName);
    if (ext.MakeLower() == "webp") {

        uint8_t* dataRaw = nullptr;
        std::unique_ptr<uint8_t> data;
        size_t dataSize = 0;
        if (!ExUtilReadFile(fileName, &dataRaw, &dataSize)) {
            return nullptr;
        }
        data.reset(dataRaw);
        WebPPic pic;
        memset(&pic, 0, sizeof(pic));
        if (!ReadWebP(data.get(), dataSize, &pic)) {
            return nullptr;
        }
        bm.reset(new Bitmap(pic.width, pic.height, PixelFormat32bppARGB));

        BitmapData dstData;
        Rect rc(0, 0, pic.width, pic.height);

        if (bm->LockBits(&rc, ImageLockModeWrite, PixelFormat32bppARGB, &dstData) == Ok) {
            uint8_t* dstBits = reinterpret_cast<uint8_t *>(dstData.Scan0);
            memcpy(dstBits, pic.rgba, pic.height * pic.width * 4);

            bm->UnlockBits(&dstData);
        }
        if (pic.animated) {
            delete[] pic.rgba;
        } else {
            WebPFree(pic.rgba);
        }

    } else {
        bm.reset(new Gdiplus::Bitmap(fileName));
    }
    Gdiplus::Status status = bm->GetLastStatus();
    if (bm && status != Gdiplus::Ok) {
        int lastError = GetLastError();

        CString error = GdiplusStatusToString(status);

        if (status == Gdiplus::Win32Error) {
            error += L"\r\n" + WinUtils::FormatWindowsErrorMessage(lastError);
        }
        ServiceLocator::instance()->logger()->write(logWarning, _T("Image Loader"), _T("Cannot load image.") + CString(L"\r\n") + error, CString(_T("File:")) + _T(" ") + fileName);
        return nullptr;
    }

    short orient = GetImageOrientation(bm.get()); 
    RotateAccordingToOrientation(orient, bm.get(), true);
    return bm;
}

using PropertyItemPtr = std::unique_ptr<PropertyItem, void(*)(PropertyItem*)>;

PropertyItemPtr GetPropertyItemFromImage(Gdiplus::Image* bm, PROPID propId) {
    auto deleter = [](PropertyItem* ptr) {
        delete[] reinterpret_cast<uint8_t*>(ptr);
    };
    UINT itemSize = bm->GetPropertyItemSize(propId);
    if (!itemSize) {
        return PropertyItemPtr(nullptr, deleter);
    }
    PropertyItemPtr item(reinterpret_cast<PropertyItem*>(new uint8_t[itemSize]), deleter);
    if (bm->GetPropertyItem(propId, itemSize, item.get()) != Ok) {
        return PropertyItemPtr(nullptr, deleter);
    }
    return item;
}

UINT VoidToInt(void* data, unsigned int size) {
    switch (size) {
        case 8:
            return *reinterpret_cast<UINT*>(data);
        case 4:
            return *reinterpret_cast<DWORD*>(data);
        case 2:
            return *reinterpret_cast<WORD*>(data);
        default:
            return *reinterpret_cast<BYTE*>(data);
    }
}

typedef IStream * (STDAPICALLTYPE *SHCreateMemStreamFuncType)(const BYTE *pInit, UINT cbInit);
SHCreateMemStreamFuncType SHCreateMemStreamFunc = 0;

Library shlwapiLib(L"Shlwapi.dll");

CComPtr<IStream>  CreateMemStream(const BYTE* pInit, UINT cbInit) {
    CComPtr<IStream> res;
    if (!SHCreateMemStreamFunc) {
        SHCreateMemStreamFunc = shlwapiLib.GetProcAddress<SHCreateMemStreamFuncType>(WinUtils::IsVistaOrLater() ? "SHCreateMemStream" : MAKEINTRESOURCEA(12));
        if (!SHCreateMemStreamFunc) {
            return res;
        }
    }
    res.Attach(SHCreateMemStreamFunc(pInit, cbInit));
    return res;
}

std::unique_ptr<Gdiplus::Bitmap> BitmapFromMemory(BYTE* data, size_t imageSize) {
    if (WinUtils::IsVistaOrLater()) {
        if (!SHCreateMemStreamFunc) {
            SHCreateMemStreamFunc = shlwapiLib.GetProcAddress<SHCreateMemStreamFuncType>("SHCreateMemStream");
            if (!SHCreateMemStreamFunc) {
                return 0;
            }
        }

        std::unique_ptr<Gdiplus::Bitmap> bitmap;
        IStream* pStream = SHCreateMemStreamFunc(data, imageSize);
        if (pStream) {
            bitmap.reset(Gdiplus::Bitmap::FromStream(pStream));
            pStream->Release();
            if (bitmap) {
                if (bitmap->GetLastStatus() == Gdiplus::Ok) {
                    return bitmap;
                }
            }
        }
    } else {
        HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
        if (buffer) {
            void* pBuffer = ::GlobalLock(buffer);
            if (pBuffer) {
                std::unique_ptr<Gdiplus::Bitmap> bitmap;
                CopyMemory(pBuffer, data, imageSize);

                IStream* pStream = nullptr;
                if (::CreateStreamOnHGlobal(buffer, FALSE, &pStream) == S_OK) {
                    bitmap.reset(Gdiplus::Bitmap::FromStream(pStream));
                    pStream->Release();
                    if (bitmap) {
                        if (bitmap->GetLastStatus() == Gdiplus::Ok) {
                            return bitmap;
                        }
                    }
                }
                ::GlobalUnlock(buffer);
            }
            ::GlobalFree(buffer);
        }
    }

    return nullptr;
}

// Based on original method from http://danbystrom.se/2009/01/05/imagegetthumbnailimage-and-beyond/
std::unique_ptr<Gdiplus::Bitmap> GetThumbnail(Gdiplus::Image* bm, int width, int height, Gdiplus::Size* realSize) {
    using namespace Gdiplus;
    if (realSize) {
        realSize->Width = bm->GetWidth();
        realSize->Height = bm->GetHeight();
    }
    Size sz = AdaptProportionalSize(Size(width, height), Size(bm->GetWidth(), bm->GetHeight()));
    std::unique_ptr<Bitmap> res = std::make_unique<Bitmap>(sz.Width, sz.Height);
    Graphics gr(res.get());

    gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    UINT size = bm->GetPropertyItemSize(PropertyTagThumbnailData);
    if (size) {
        // Loading thumbnail from EXIF data (fast)
        enum ThumbCompression { ThumbCompressionJPEG, ThumbCompressionRGB, ThumbCompressionYCbCr, ThumbCompressionUnknown }
            compression = ThumbCompressionJPEG;

        short orient = 0;
        auto orientationItem = GetPropertyItemFromImage(bm, PropertyTagOrientation);
        if (orientationItem) {
            memcpy(&orient, orientationItem->value, sizeof(orient));
        }
        auto thumbnailFormatItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailFormat);
        if (thumbnailFormatItem) {
            UINT format = VoidToInt(thumbnailFormatItem->value, thumbnailFormatItem->length);
            if (format == 0) {
                compression = ThumbCompressionRGB;
            } else if (format == 1) {
                compression = ThumbCompressionJPEG;
            } else {
                compression = ThumbCompressionUnknown;
            }
        } else {
            auto compressionItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailCompression);
            if (compressionItem) {
                WORD compressionTag = *reinterpret_cast<WORD*>(compressionItem->value);
                if (compressionTag == 1) {
                    compression = ThumbCompressionRGB;
                    auto photometricInterpretationItem = GetPropertyItemFromImage(bm, PropertyTagPhotometricInterp);
                    if (photometricInterpretationItem) {
                        UINT photoMetricInterpretationTag = VoidToInt(photometricInterpretationItem->value, photometricInterpretationItem->length);
                        if (photoMetricInterpretationTag == 6) {
                            compression = ThumbCompressionYCbCr;
                        }
                    }

                } else if (compressionTag == 6) {
                    compression = ThumbCompressionJPEG;
                }
            }
        }
        
        int originalThumbWidth = 0, originalThumbHeight = 0;
        if (compression == ThumbCompressionJPEG || compression == ThumbCompressionRGB) {
            auto thumbDataItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailData);
            if (thumbDataItem) {
                if (compression == ThumbCompressionJPEG) {
                    std::unique_ptr<Bitmap> src(BitmapFromMemory(reinterpret_cast<BYTE*>(thumbDataItem->value), thumbDataItem->length));
                    if (src) {
                        RotateAccordingToOrientation(orient, src.get());

//                        int ww = src->GetWidth();
//                        int hh = src->GetHeight();
                        gr.DrawImage(src.get(), 0, 0, sz.Width, sz.Height);
                        return res;
                    }
                } else if (compression == ThumbCompressionRGB) {
                    auto widthItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailImageWidth);
                    if (widthItem) {
                        originalThumbWidth = VoidToInt(widthItem->value, widthItem->length);
                    }
                    auto heightItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailImageHeight);
                    if (heightItem) {
                        originalThumbHeight = VoidToInt(heightItem->value, heightItem->length);
                    }
                    if (originalThumbWidth && originalThumbHeight) {
                        BITMAPINFOHEADER bih;
                        memset(&bih, 0, sizeof(bih));
                        bih.biSize = sizeof(bih);
                        bih.biWidth = originalThumbWidth;
                        bih.biHeight = -originalThumbHeight;
                        bih.biPlanes = 1;
                        bih.biBitCount = 24;

                        BITMAPINFO bi;
                        memset(&bi, 0, sizeof(bi));
                        bi.bmiHeader = bih;

                        BYTE* data = reinterpret_cast<BYTE*>(thumbDataItem->value);
                        BYTE temp;
                        // Convert RGB to BGR
                        for (unsigned int offset = 0; offset < thumbDataItem->length; offset += 3) {
                            temp = data[offset];
                            data[offset] = data[offset + 2];
                            data[offset + 2] = temp;
                        }
                        Bitmap src(&bi, thumbDataItem->value);

                        if (src.GetLastStatus() == Ok) {
                            gr.DrawImage(&src, 0, 0, sz.Width, sz.Height);
                            return res;
                        }
                        
                    }

                } else {
                    // other type of compression not implemented
                }
            }
        }
    } 
    // Fallback - Load full image and draw it  (slow)
    gr.DrawImage(bm, 0, 0, sz.Width, sz.Height);

    return res;
}

std::unique_ptr<Gdiplus::Bitmap> GetThumbnail(const CString& filename, int width, int height, Gdiplus::Size* realSize) {
    using namespace Gdiplus;
    std::unique_ptr<Bitmap> bm (LoadImageFromFileExtended(filename));
    if (!bm) {
        return nullptr;
    }
    //Image bm(filename);
    if (bm->GetLastStatus() != Ok) {
        return nullptr;
    }
    return GetThumbnail(bm.get(), width, height, realSize);
}

Gdiplus::Size AdaptProportionalSize(const Gdiplus::Size& szMax, const Gdiplus::Size& szReal)
{
    int nWidth;
    int nHeight;
    double sMaxRatio;
    double sRealRatio;

    if (szMax.Width < 1 || szMax.Height < 1 || szReal.Width < 1 || szReal.Height < 1)
        return Size();

    sMaxRatio = szMax.Width / static_cast<double>(szMax.Height);
    sRealRatio = szReal.Width / static_cast<double>(szReal.Height);

    if (sMaxRatio < sRealRatio) {
        nWidth = min(szMax.Width, szReal.Width);
        nHeight = static_cast<int>(round(nWidth / sRealRatio));
    } else {
        nHeight = min(szMax.Height, szReal.Height);
        nWidth = static_cast<int>(round(nHeight * sRealRatio));
    }

    return Size(nWidth, nHeight);
}

/*
void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Font &font,Color &ColorText,Color &ColorStroke,int HorPos,int VertPos, int width)
{
RectF OriginalTextRect;
//    , NewTextRect;
FontFamily ff;
font.GetFamily(&ff);

gr.SetPageUnit(UnitPixel);
gr.MeasureString(Text,-1,&font,PointF(0,0),&OriginalTextRect);



Font NewFont(&ff,48,font.GetStyle(),UnitPixel);
Rect realSize = MeasureDisplayString(gr, Text, RectF(0,0,1000,200), NewFont);

GraphicsPath path;
StringFormat format;

//format.SetAlignment(StringAlignmentCenter);
//format.SetLineAlignment ( StringAlignmentCenter);
StringFormat * f = format.GenericTypographic()->Clone();
f->SetTrimming(StringTrimmingNone);
f->SetFormatFlags( StringFormatFlagsMeasureTrailingSpaces);

path.AddString(Text, -1,&ff, (int)NewFont.GetStyle(), NewFont.GetSize(), Point(0,0), f);
RectF realTextBounds;
path.GetBounds(&realTextBounds);

float k = 2*width*realTextBounds.Height/OriginalTextRect.Height;
Pen pen(ColorStroke,(float)k);
realTextBounds.Inflate(k, k);
pen.SetAlignment(PenAlignmentCenter);
//path.GetBounds(&realTextBounds,0,&pen);
k = 2*width*realTextBounds.Height/OriginalTextRect.Height; // recalculate K
//Matrix matrix(1.0f, 0.0f, 0.0f, 1.0f, -realTextBounds.X,-realTextBounds.Y );
//path.Transform(&matrix);
//path.GetBounds(&realTextBounds,0,&pen);
//gr.SetPageUnit(UnitPixel);


//Font NewFont(&ff,48,font.GetStyle(),UnitPixel);

RectF NewTextRect;

gr.MeasureString(Text,-1,&NewFont,RectF(0,0,5000,1600),f, &NewTextRect);

RectF RectWithPaddings;
StringFormat f2;
gr.MeasureString(Text,-1,&NewFont,RectF(0,0,5000,1600),&f2, &RectWithPaddings);


OriginalTextRect.Height = OriginalTextRect.Height-OriginalTextRect.Y;
float newwidth,newheight;
newheight = OriginalTextRect.Height;
newwidth=OriginalTextRect.Height/realTextBounds.Height*realTextBounds.Width;

SolidBrush br(ColorText);

Bitmap temp((int)(realTextBounds.Width+realTextBounds.X),(int)(realTextBounds.Height+realTextBounds.Y),&gr);
//Bitmap temp((int)NewTextRect.Width,(int)NewTextRect.Height,&gr);

Graphics gr_temp(&temp);

gr_temp.SetPageUnit(UnitPixel);
//    GraphicsPath path;
gr_temp.SetSmoothingMode( SmoothingModeHighQuality);
//path.AddString(Text, -1,&ff, (int)NewFont.GetStyle(), NewFont.GetSize(), Point(0,0), &format);
gr_temp.SetSmoothingMode( SmoothingModeHighQuality);


SolidBrush br2(Color(255,0,0));
float x,y;
//gr_temp.FillRectangle(&br2, (int)realTextBounds.X, (int)realTextBounds.Y,(int)(realTextBounds.Width),(int)(realTextBounds.Height));
//gr_temp.Set( SmoothingModeHighQuality);
gr_temp.SetInterpolationMode(InterpolationModeHighQualityBicubic  );
gr_temp.DrawPath(&pen, &path);
gr_temp.FillPath(&br, &path);

gr.SetSmoothingMode( SmoothingModeHighQuality);
gr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );

if(HorPos == 0)
x = 2;
else if(HorPos == 1)
x = (Bounds.Width-newwidth)/2;
else x=(Bounds.Width-newwidth)-2;

if(VertPos==0)
y=2;
else if(VertPos==1)
y=(Bounds.Height-newheight)/2;
else y=(Bounds.Height-newheight)-2;


Rect destRect ((int)(Bounds.GetLeft()+x), (int)(Bounds.GetTop()+y), (int)(newwidth),(int)(newheight));
//    gr.SolidBrush br3(Color(0,255,0));

//gr.FillRectangle(&br2, 0,0,(int)(realTextBounds.Width+realTextBounds.X),(int)(realTextBounds.Height+realTextBounds.Y));

gr.DrawImage(&temp, destRect,(int)realTextBounds.X, (int)realTextBounds.Y,(int)(realTextBounds.Width),(int)(realTextBounds.Height), UnitPixel);
}*/

bool CopyDataToClipboardInDataUriFormat(ULONGLONG dataSize, std::string mimeType, bool html, std::function<size_t(void*, size_t)> readCallback) {
    ULONGLONG offset = 0;
    size_t leftBytes = static_cast<size_t>(dataSize);
    const ULONG buf_size = 1024 * 32;
    char buffer[buf_size];

    const char* footer = "\" alt=\"\" />";
    int footerLen = strlen(footer);
    const char* head = "<img src=\"";
    int headLen = strlen(head);

    HGLOBAL hglbCopy;
    int cch = static_cast<int>(dataSize * 4 / 3 + 40);
    if (html) {
        cch += footerLen + headLen;
    }
    if (!OpenClipboard(NULL)) {
        LOG(ERROR) << "Failed to open clipboard";
        return FALSE;
    }

    EmptyClipboard();
    size_t bytesToAlloc = (cch + 1) * sizeof(char);
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, bytesToAlloc);
    if (hglbCopy == NULL) {
        LOG(ERROR) << "Failed to alloc global memory (" << bytesToAlloc << " bytes).";
        CloseClipboard();
        return FALSE;
    }

    base64_state state;
    base64_stream_encode_init(&state, 0);
    char* encodedData = reinterpret_cast<char*>(GlobalLock(hglbCopy));
    char* encodedDataCur = encodedData;

    if (html) {

        strncpy(encodedDataCur, head, headLen);
        encodedDataCur += headLen;
    }
    strncpy(encodedDataCur, "data:", 5);
    encodedDataCur += 5;
   
    strncpy(encodedDataCur, mimeType.c_str(), mimeType.length());
    encodedDataCur += mimeType.length();
    strncpy(encodedDataCur, ";base64,", 8);
    encodedDataCur += 8;
    size_t outlen = 0;
    while (offset < dataSize) {
        size_t readBytes = readCallback(buffer, std::min<size_t>(buf_size, leftBytes));
        if (!readBytes) {
            break;
        }
        leftBytes -= readBytes;
        offset += readBytes;
        outlen = 0;
        base64_stream_encode(&state, buffer, readBytes, encodedDataCur, &outlen);
        encodedDataCur += outlen;
    }
    base64_stream_encode_final(&state, encodedDataCur, &outlen);
    encodedDataCur += outlen;
    if (html) {
        strncpy(encodedDataCur, footer, footerLen);
        encodedDataCur += footerLen;
    }

    *encodedDataCur = 0;

    GlobalUnlock(hglbCopy);
    SetClipboardData(CF_TEXT, hglbCopy);

    if (html) {
        std::string clipboardHtml = WinUtils::TextToClipboardHtmlFormat(encodedData, encodedDataCur - encodedData);
        unsigned htmlClipboardFormatId = RegisterClipboardFormat(_T("HTML Format"));
        size_t clipboardHtmlSize = clipboardHtml.size();
        HGLOBAL hglbHtml = GlobalAlloc(GMEM_MOVEABLE, clipboardHtmlSize + 1);
        char* htmlData = reinterpret_cast<char*>(GlobalLock(hglbHtml));

        strncpy(htmlData, clipboardHtml.c_str(), clipboardHtmlSize);
        htmlData[clipboardHtmlSize] = 0;
        GlobalUnlock(hglbHtml);
        SetClipboardData(htmlClipboardFormatId, hglbHtml);
    }
    CloseClipboard();
    return true;
}

bool CopyFileToClipboardInDataUriFormat(const CString& fileName, int Format, int Quality, bool html) {
    std::string fileNameUtf8 = W2U(fileName);
    std::string mimeType = IuCoreUtils::GetFileMimeType(fileNameUtf8);
    FILE* f = IuCoreUtils::fopen_utf8(fileNameUtf8.c_str(), "rb");
    if (!f) {
        LOG(ERROR) << boost::format("Could not save xml to file '%s'.") % fileName << std::endl << "Reason: " << strerror(errno);
        return false;
    }
    int64_t fileSize = IuCoreUtils::getFileSize(fileNameUtf8);
    if (fileSize > 10 * 1024 * 1024) {
        LOG(ERROR) << "File is too big"; 
        return false;
    }
    
    bool res = CopyDataToClipboardInDataUriFormat(fileSize, mimeType, html, [&](void* buffer, size_t size) -> size_t {
        if (feof(f)) {
            return 0;
        }
        return fread(buffer, 1, size, f);
    });
    fclose(f);
    return res;
}

bool CopyBitmapToClipboardInDataUriFormat(Bitmap* bm, int Format, int Quality, bool html) {
    CComPtr<IStream> stream = CreateMemStream(nullptr, 0);
    CString mimeType;
    bool res = SaveImageToFile(bm, CString(), stream, Format, Quality, &mimeType);
    std::string mimeTypeA = W2U(mimeType);
    STATSTG stats;
    HRESULT hr = stream->Stat(&stats, STATFLAG_NONAME);
    if (SUCCEEDED(hr)) {
        hr = stream->Seek({ 0, 0 }, STREAM_SEEK_SET, nullptr);
        if (SUCCEEDED(hr)) {
            CopyDataToClipboardInDataUriFormat(stats.cbSize.QuadPart, mimeTypeA, html, [&](void* buffer, size_t size) -> size_t {
                unsigned long readBytes;
                HRESULT hr = stream->Read(buffer, size, &readBytes);
                if (!SUCCEEDED(hr)) {
                    return 0;
                }
                return readBytes;
            });
        }
    }
    return res;

}


ImageInfo GetImageInfo(const wchar_t* fileName) {
    std::unique_ptr<Bitmap> bm(LoadImageFromFileExtended(fileName));
    ImageInfo res;
    if (bm) {
        res.width = bm->GetWidth();
        res.height = bm->GetHeight();
    }
    return res;
}

}