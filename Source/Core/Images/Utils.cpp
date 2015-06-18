/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "3rdpart/GdiplusH.h"
#include "Core/Logging.h"
#include "Func/WinUtils.h"
#include "3rdpart/QColorQuantizer.h"
#include "Core/Utils/StringUtils.h"
#include "Core/Utils/CoreUtils.h"
#include "Func/MyUtils.h"
#include "Func/IuCommonFunctions.h"
#include "Func/Common.h"
#include "Gui/Dialogs/LogWindow.h"
#include <math.h>
#include <stdint.h>
#include <Core/ServiceLocator.h>


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

    double anInchX = 1440.0 / GetDeviceCaps(hdc1, LOGPIXELSX);
    double anInchY = 1440.0 / GetDeviceCaps(hdc1, LOGPIXELSY);
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
    gr2.DrawImage(background,layoutArea.GetLeft(),layoutArea.GetTop(),layoutArea.GetLeft(), layoutArea.GetTop(), layoutArea.Width, layoutArea.Height,Gdiplus::UnitPixel/*gr2.GetPageUnit()*/);

    FORMATRANGE fmtRange;
    fmtRange.chrg.cpMax = -1;                    //Indicate character from to character to 
    fmtRange.chrg.cpMin = 0;
    fmtRange.hdc = hdc;                                //Use the same DC for measuring and rendering
    fmtRange.hdcTarget = hdc;                    //Point at printer hDC
    fmtRange.rc = rectLayoutArea;            //Indicate the area on page to print
    fmtRange.rcPage = rectLayoutArea;    //Indicate size of page


    /*int characterCount = */::SendMessage(hwnd, EM_FORMATRANGE, 1, (LPARAM)&fmtRange);

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
    gp.AddLine(r.X, max(r.Y, r.Y + r.Height - d), r.X, min(r.Y + d/2, r.GetBottom()));
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

    if (st != Gdiplus::Ok) {
        return 0;
    }

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
            val += scl[ri++] - fv       ;   
            tcl[ti++] = static_cast<uint8_t>(round(val*iarr)); 
        }
        for(int j=r+1; j<w-r; j++) { val += scl[ri++] - scl[li++];   tcl[ti++] = round(val*iarr); }
        for(int j=w-r; j<w  ; j++) { val += lv        - scl[li++];   tcl[ti++] = round(val*iarr); }
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
    //    delete[] buf2;
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

    BYTE* bpSrc = (BYTE*)bdSrc.Scan0;

    //bpSrc += (int)sourceChannel;


    for ( int i = r.Height * r.Width; i > 0; i-- )
    {
        BGRA_COLOR * c = (BGRA_COLOR *)bpSrc;

        if(c->a!=255)
        {
            //c = 255;

            DWORD * d= (DWORD*)bpSrc;
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
    Bitmap temp((int)NewTextRect.Width, (int)NewTextRect.Height, &gr);

    Graphics gr_temp(&temp);
    StringFormat format;
    gr_temp.SetPageUnit(UnitPixel);
    GraphicsPath path;
    gr_temp.SetSmoothingMode(SmoothingModeHighQuality);
    path.AddString(Text, -1, &ff, (int)NewFont.GetStyle(), NewFont.GetSize(), Point(0, 0), &format);

    Pen pen(ColorStroke, (float)k);
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

    gr.DrawImage(&temp, (int)(Bounds.GetLeft() + x), (int)(Bounds.GetTop() + y), (int)(newwidth), (int)(newheight));
}


// hack for stupid GDIplus
void changeAplhaChannel(Bitmap& source, Bitmap& dest, int sourceChannel, int destChannel)
{
    Rect r(0, 0, source.GetWidth(), source.GetHeight());
    BitmapData bdSrc;
    BitmapData bdDst;
    source.LockBits(&r, ImageLockModeRead, PixelFormat32bppARGB, &bdSrc);
    dest.LockBits(&r, ImageLockModeWrite, PixelFormat32bppARGB, &bdDst);

    BYTE* bpSrc = (BYTE*)bdSrc.Scan0;
    BYTE* bpDst = (BYTE*)bdDst.Scan0;
    bpSrc += (int)sourceChannel;
    bpDst += (int)destChannel;

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
    std::auto_ptr<Bitmap> quantizedImage;
    TCHAR szImgTypes[3][4] = { _T("jpg"), _T("png"), _T("gif") };
    TCHAR szMimeTypes[3][12] = { _T("image/jpeg"), _T("image/png"), _T("image/gif") };
    CString szNameBuffer;
    TCHAR szBuffer2[MAX_PATH];
    if (IsImage(szFilename))
        szNameBuffer = GetOnlyFileName(szFilename);
    else
        szNameBuffer = szFilename;
    CString userFolder;
    if (Folder)
        userFolder = Folder;
    if (userFolder.Right(1) != _T('\\'))
        userFolder += _T('\\');
    wsprintf(szBuffer2, _T(
        "%s%s.%s"), (LPCTSTR)(Folder ? userFolder : IuCommonFunctions::IUTempFolder), (LPCTSTR)szNameBuffer,
        /*(int)GetTickCount(),*/ szImgTypes[Format]);
    CString resultFilename = WinUtils::GetUniqFileName(szBuffer2);
    IU_CreateFilePath(resultFilename);
    CLSID clsidEncoder;
    EncoderParameters eps;
    eps.Count = 1;

    if (Format == 0) // JPEG
    {
        eps.Parameter[0].Guid = EncoderQuality;
        eps.Parameter[0].Type = EncoderParameterValueTypeLong;
        eps.Parameter[0].NumberOfValues = 1;
        eps.Parameter[0].Value = &Quality;
    }
    else
        if (Format == 1)      // PNG
        {
            eps.Parameter[0].Guid = EncoderCompression;
            eps.Parameter[0].Type = EncoderParameterValueTypeLong;
            eps.Parameter[0].NumberOfValues = 1;
            eps.Parameter[0].Value = &Quality;
        }
        else
            if (Format == 2)
            {
                QColorQuantizer quantizer;
                quantizedImage.reset(quantizer.GetQuantized(img, QColorQuantizer::Octree, (Quality < 50) ? 16 : 256));
                if (quantizedImage.get() != 0)
                    img = quantizedImage.get();
            }

    Gdiplus::Status result;

    if (GetEncoderClsid(szMimeTypes[Format], &clsidEncoder) != -1)
    {
        if (Format == 0)
            result = img->Save(resultFilename, &clsidEncoder, &eps);
        else
            result = img->Save(resultFilename, &clsidEncoder);
    }
    else
        return false;
    if (result != Ok)
    {
        ServiceLocator::instance()->logger()->write(logError, _T("Image Converter"), _T("Could not save image at path \r\n") + resultFilename + L"\r\nGdiplus status=" + WinUtils::IntToStr(result));
        return false;
    }
    szBuffer = resultFilename;
    return true;
}

CRect CenterRect(CRect r1, CRect intoR2)
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

PropertyItem* GetPropertyItemFromImage(Gdiplus::Image* bm, PROPID propId) {
    UINT itemSize = bm->GetPropertyItemSize(propId);
    if (!itemSize) {
        return 0;
    }
    PropertyItem* item = reinterpret_cast<PropertyItem*>(malloc(itemSize));
    if (bm->GetPropertyItem(propId, itemSize, item) != Ok) {
        free(item);
        return 0;
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

Gdiplus::Bitmap* BitmapFromMemory(BYTE* data, size_t imageSize) {
    if (WinUtils::IsVista()) {
        if (!SHCreateMemStreamFunc) {
            HMODULE lib = LoadLibrary(_T("Shlwapi.dll"));
            SHCreateMemStreamFunc = reinterpret_cast<SHCreateMemStreamFuncType>(GetProcAddress(lib, "SHCreateMemStream"));
            if (!SHCreateMemStreamFunc) {
                return 0;
            }
        }

        Gdiplus::Bitmap * bitmap;
        IStream* pStream = SHCreateMemStreamFunc(data, imageSize);
        if (pStream) {
            bitmap = Gdiplus::Bitmap::FromStream(pStream);
            pStream->Release();
            if (bitmap) {
                if (bitmap->GetLastStatus() == Gdiplus::Ok) {
                    return bitmap;
                }
                delete bitmap;
            }
        }
    } else {
        HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
        if (buffer) {
            void* pBuffer = ::GlobalLock(buffer);
            if (pBuffer) {
                Gdiplus::Bitmap * bitmap;
                CopyMemory(pBuffer, data, imageSize);

                IStream* pStream = NULL;
                if (::CreateStreamOnHGlobal(buffer, FALSE, &pStream) == S_OK) {
                    bitmap = Gdiplus::Bitmap::FromStream(pStream);
                    pStream->Release();
                    if (bitmap) {
                        if (bitmap->GetLastStatus() == Gdiplus::Ok) {
                            return bitmap;
                        }

                        delete bitmap;
                    }
                }
                ::GlobalUnlock(buffer);
            }
            ::GlobalFree(buffer);
        }
    }

    return 0;
}

// Based on original method from http://danbystrom.se/2009/01/05/imagegetthumbnailimage-and-beyond/
Gdiplus::Bitmap* GetThumbnail(Gdiplus::Image* bm, int width, int height, Gdiplus::Size* realSize) {
    using namespace Gdiplus;
    if (realSize) {
        realSize->Width = bm->GetWidth();
        realSize->Height = bm->GetHeight();
    }
    Size sz = AdaptProportionalSize(Size(width, height), Size(bm->GetWidth(), bm->GetHeight()));
    Bitmap* res = new Bitmap(sz.Width, sz.Height);
    Graphics gr(res);

    gr.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    UINT size = bm->GetPropertyItemSize(PropertyTagThumbnailData);
    if (size) {
        // Loading thumbnail from EXIF data (fast)
        enum ThumbCompression { ThumbCompressionJPEG, ThumbCompressionRGB, ThumbCompressionYCbCr, ThumbCompressionUnknown }
            compression = ThumbCompressionJPEG;

        PropertyItem* thumbnailFormatItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailFormat);
        if (thumbnailFormatItem) {
            UINT format = VoidToInt(thumbnailFormatItem->value, thumbnailFormatItem->length);
            if (format == 0) {
                compression = ThumbCompressionRGB;
            } else if (format == 1) {
                compression = ThumbCompressionJPEG;
            } else {
                compression = ThumbCompressionUnknown;
            }
            free(thumbnailFormatItem);
        } else {
            PropertyItem* compressionItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailCompression);
            if (compressionItem) {
                WORD compressionTag = *reinterpret_cast<WORD*>(compressionItem->value);
                if (compressionTag == 1) {
                    compression = ThumbCompressionRGB;
                    PropertyItem* photometricInterpretationItem = GetPropertyItemFromImage(bm, PropertyTagPhotometricInterp);
                    if (photometricInterpretationItem) {
                        UINT photoMetricInterpretationTag = VoidToInt(photometricInterpretationItem->value, photometricInterpretationItem->length);
                        free(photometricInterpretationItem);
                        if (photoMetricInterpretationTag == 6) {
                            compression = ThumbCompressionYCbCr;
                        }
                    }

                } else if (compressionTag == 6) {
                    compression = ThumbCompressionJPEG;
                }

                free(compressionItem);
            }
        }
        
        int originalThumbWidth = 0, originalThumbHeight = 0;
        if (compression == ThumbCompressionJPEG || compression == ThumbCompressionRGB) {
            PropertyItem* thumbDataItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailData);
            if (thumbDataItem) {
                if (compression == ThumbCompressionJPEG) {
                    Bitmap* src = BitmapFromMemory(reinterpret_cast<BYTE*>(thumbDataItem->value), thumbDataItem->length);
                   
                    if (src) {
                        gr.DrawImage(src, 0, 0, sz.Width, sz.Height);
                        delete src;
                        free(thumbDataItem);
                        return res;
                    }
                } else if (compression == ThumbCompressionRGB) {
                    PropertyItem* widthItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailImageWidth);
                    if (widthItem) {
                        originalThumbWidth = VoidToInt(widthItem->value, widthItem->length);
                        free(widthItem);
                    }
                    PropertyItem* heightItem = GetPropertyItemFromImage(bm, PropertyTagThumbnailImageHeight);
                    if (heightItem) {
                        originalThumbHeight = VoidToInt(heightItem->value, heightItem->length);
                        free(heightItem);
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
                            free(thumbDataItem);
                            return res;
                        }
                        
                    }

                } else {
                    // other type of compression not implemented
                }
                free(thumbDataItem);
            }
        }
    } 
    // Fallback - Load full image and draw it  (slow)
    gr.DrawImage(bm, 0, 0, sz.Width, sz.Height);

    return res;
}

Gdiplus::Bitmap* GetThumbnail(const CString& filename, int width, int height, Gdiplus::Size* realSize) {
    using namespace Gdiplus;
    Image bm(filename);
    if (bm.GetLastStatus() != Ok) {
        return 0;
    }
    return GetThumbnail(&bm, width, height, realSize);
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