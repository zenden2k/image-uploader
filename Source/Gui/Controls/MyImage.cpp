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
    IsImage = false;
    BackBufferDc = NULL;
    BackBufferBm = 0;
    HideParent = false;
    ImageWidth  = 0;
    ImageHeight = 0;

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
    PAINTSTRUCT ps;
    HDC hdc;
    bHandled = true;
    if (!wParam) {
        BeginPaint(&ps);
        hdc = ps.hdc;
    } else {
        hdc = BeginPaint(&ps);
    }

    if (IsImage)
    {
        RECT rc;
        GetClientRect(&rc);
        SetBkMode(hdc, TRANSPARENT);
        BitBlt(hdc, 0, 0, BackBufferWidth, BackBufferHeight, BackBufferDc, 0, 0, SRCCOPY);
    }

    if (!wParam) {
        EndPaint(&ps);
    }
    bHandled = true;
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

bool CMyImage::LoadImage(LPCTSTR FileName, Image* img, int ResourceID, bool Bmp, COLORREF transp)
{
    RECT rc;
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

    float width, height, imgwidth, imgheight, newwidth = 0, newheight=0;
    width = static_cast<float>(rc.right);
    height = static_cast<float>(rc.bottom);

    if (!ResourceID)
    {
        width -=  2;
        height -=  2;
    }

    CWindowDC dc(m_hWnd);

    BackBufferDc = ::CreateCompatibleDC(dc);
    BackBufferBm = ::CreateCompatibleBitmap(dc, BackBufferWidth, BackBufferHeight);

    oldBm_ = ::SelectObject(BackBufferDc, BackBufferBm);

    Graphics gr(BackBufferDc);

    std::unique_ptr<Image> newBm;
    Image* bm = nullptr;
    bool WhiteBg = false;
    if (img) {
        bm = img;
    } else if (FileName) {
        std::unique_ptr<GdiPlusImage> srcImg = ImageUtils::LoadImageFromFileExtended(FileName);
        if (srcImg) {
            newBm = std::unique_ptr<Image>(srcImg->releaseBitmap());
        }
    }
    else if (ResourceID){
        if (!Bmp)
        {
            newBm = ImageUtils::BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(ResourceID), _T("PNG"));
            WhiteBg = true;
        }
        else
        {
            newBm = std::make_unique<Bitmap>(GetModuleHandle(0), MAKEINTRESOURCE(ResourceID));
        }
    }

    if (newBm) {
        bm = newBm.get();
    }

    if (bm)
    {
        imgwidth = static_cast<float>(bm->GetWidth());
        imgheight = static_cast<float>(bm->GetHeight());
        ImageWidth = static_cast<int>(imgwidth);
        ImageHeight = static_cast<int>(imgheight);
        if (imgwidth / imgheight > width / height)
        {
            if (imgwidth <= width)
            {
                newwidth = imgwidth;
                newheight = imgheight;
            }
            else
            {
                newwidth = width;
                newheight = newwidth / imgwidth * imgheight;
            }
        }
        else
        {
            if (imgheight <= height)
            {
                newwidth = imgwidth;
                newheight = imgheight;
            }
            else
            {
                newheight = height;
                newwidth = (newheight / imgheight) * imgwidth;
            }
        }
    }

    if (ResourceID)
    {
        newwidth = static_cast<float>(BackBufferWidth);
        newheight = static_cast<float>(BackBufferHeight);
    }
    gr.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    if (WhiteBg)
        gr.Clear(Color(GetRValue(transp), GetGValue(transp), GetBValue(transp)));

    else
        gr.Clear(Color(255, 145, 145, 145));

    RectF bounds(1, 1, float(width), float(height));
    if ((bm) && !bm->GetWidth() && (FileName || ResourceID) )
    {
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
    }

    else
    {
        SolidBrush bb(Color(255, 255, 255, 255));

        LinearGradientBrush
        br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210),
           LinearGradientModeBackwardDiagonal);

        if (!WhiteBg)
            gr.FillRectangle(&br, (float)1, (float)1, (float)width, (float)height);

        IsImage = true;
        if (bm)
            gr.DrawImage(bm, (int)((ResourceID) ? 0 : 1 + (width - newwidth) / 2),
                         (int)((ResourceID) ? 0 : 1 + (height - newheight) / 2), (int)newwidth, (int)newheight);
    }

    IsImage = true;
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


