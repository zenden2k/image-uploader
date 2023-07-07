/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include "MyImage.h"

#include "Core/i18n/Translator.h"
#include "Core/Images/Utils.h"

using namespace Gdiplus;
// CMyImage
CMyImage::CMyImage() : bm_(nullptr), BackBufferWidth(0), BackBufferHeight(0)
{
    imageLoaded_ = false;
    BackBufferDc = NULL;
    BackBufferBm = 0;
    HideParent = false;
    imageWidth_  = 0;
    imageHeight_ = 0;

}

CMyImage::~CMyImage()
{
    if (BackBufferDc) {
        SelectObject(BackBufferDc, oldBm_);
        DeleteDC(BackBufferDc);
    }
    if (BackBufferBm) {
        DeleteObject(BackBufferBm);
    }
}

LRESULT CMyImage::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    CPaintDC dc(m_hWnd);

    bHandled = true;

    if (imageLoaded_)
    {
        RECT rc;
        GetClientRect(&rc);
        dc.SetBkMode(TRANSPARENT);
        dc.BitBlt(0, 0, BackBufferWidth, BackBufferHeight, BackBufferDc, 0, 0, SRCCOPY);
    }

    return 0;
}

LRESULT CMyImage::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    return 0;
}

LRESULT CMyImage::OnEraseBkg(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    bHandled = true;
    return TRUE;
}

bool CMyImage::loadImage(LPCTSTR FileName, Image* img, int ResourceID, bool Bmp, COLORREF transp, bool allowEnlarge)
{
    CRect rc;
    GetClientRect(&rc);

    if (BackBufferDc) {
        SelectObject(BackBufferDc, oldBm_);
        DeleteDC(BackBufferDc);
    }
    BackBufferDc = nullptr;
    if (BackBufferBm) {
        DeleteObject(BackBufferBm);
    }
    BackBufferBm = nullptr;
    oldBm_ = nullptr;

    Graphics g(m_hWnd, true);

    BackBufferWidth = rc.right;
    BackBufferHeight = rc.bottom;

    int imgwidth = 0, imgheight = 0;
    int newwidth = 0, newheight = 0;
    int width = rc.Width();
    int height = rc.Height();

    if (!ResourceID)
    {
        width -=  2;
        height -=  2;
    }

    CClientDC dc(m_hWnd);

    BackBufferDc = ::CreateCompatibleDC(dc);
    BackBufferBm = ::CreateCompatibleBitmap(dc, BackBufferWidth, BackBufferHeight);

    oldBm_ = ::SelectObject(BackBufferDc, BackBufferBm);

    Graphics gr(BackBufferDc);
    gr.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    gr.SetPixelOffsetMode(PixelOffsetModeHalf);
    ImageAttributes attr;
    attr.SetWrapMode(WrapModeTileFlipXY);

    std::unique_ptr<Image> newBm;
    Image* bm = nullptr;
    bool WhiteBg = false;
    if (img) {
        bm = img;
    } else if (FileName) {
        std::unique_ptr<GdiPlusImage> srcImg = ImageUtils::LoadImageFromFileExtended(FileName);
        if (srcImg) {
            newBm.reset(srcImg->releaseBitmap());
        }
    } else if (ResourceID){
        if (!Bmp) {
            newBm = ImageUtils::BitmapFromResource(_Module.GetResourceInstance(), MAKEINTRESOURCE(ResourceID), _T("PNG"));
            WhiteBg = true;
        } else {
            newBm = std::make_unique<Bitmap>(_Module.GetResourceInstance(), MAKEINTRESOURCE(ResourceID));
        }
    }

    if (newBm) {
        bm = newBm.get();
    }

    if (bm){
        imageWidth_ = imgwidth = static_cast<int>(bm->GetWidth());
        imageHeight_ = imgheight = static_cast<int>(bm->GetHeight());

        Size sz = allowEnlarge ? ImageUtils::ProportionalSize(Size(imgwidth, imgheight), Size(width, height)) :
            ImageUtils::AdaptProportionalSize(Size(width, height), Size(imgwidth, imgheight));

        newwidth = sz.Width;
        newheight = sz.Height;
    }

    if (WhiteBg) {
        gr.Clear(Color(GetRValue(transp), GetGValue(transp), GetBValue(transp)));
    } else {
        gr.Clear(Color(255, 145, 145, 145));
    } 

    RectF bounds(1, 1, float(width), float(height));
    if ((bm) && !bm->GetWidth() && (FileName || ResourceID)) {
        LinearGradientBrush
            br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210), LinearGradientModeBackwardDiagonal);
        gr.FillRectangle(&br, (float)1, (float)1, (float)width, (float)height);

        LinearGradientBrush
            brush(bounds, Color(255, 95, 95, 95), Color(255, 125, 125, 125),
                LinearGradientModeBackwardDiagonal);

        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        Font font(L"Arial", 12, FontStyleBold);

        gr.DrawString(TR("Unable to load picture"), -1, &font, bounds, &format, &brush);
    } else {
        LinearGradientBrush br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210),
           LinearGradientModeBackwardDiagonal);

        if (!WhiteBg) {
            gr.FillRectangle(&br, (float)1, (float)1, (float)width, (float)height);
        }

        imageLoaded_ = true;
        Rect destRect(ResourceID ? 0 : 1 + (width - newwidth) / 2, ResourceID ? 0 : 1 + (height - newheight) / 2, newwidth, newheight);
        if (bm) {
            gr.DrawImage(bm, destRect, 0, 0, imgwidth, imgheight, UnitPixel, &attr);
        }
    }

    imageLoaded_ = true;
    Invalidate();
    return false;
}

LRESULT CMyImage::OnLButtonDown(UINT Flags, CPoint Pt)
{
    if (HideParent) {
        ::ShowWindow(GetParent(), SW_HIDE);
    }
    return 0;
}

LRESULT CMyImage::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&) {
    return SendMessage(GetParent(), uMsg, wParam, lParam);
}
