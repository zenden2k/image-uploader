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

#include "../../atlheaders.h"
#include "MyImage.h"
#include "../Dialogs/LogWindow.h"
#include "../../Func/LangClass.h"
#include "../../Func/Settings.h"
using namespace Gdiplus;
// CMyImage
CMyImage::CMyImage()
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
	if(BackBufferDc) DeleteObject(BackBufferDc);
	if(BackBufferBm) DeleteObject(BackBufferBm);
}

LRESULT CMyImage::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	PAINTSTRUCT ps;
	bHandled = true;
	if(!wParam) BeginPaint(&ps);

	HDC hdc = wParam? (HDC)wParam:ps.hdc;

	if(IsImage)
	{
		RECT rc;
		GetClientRect(&rc);
		SetBkMode(hdc, TRANSPARENT);
		HBRUSH br = CreateSolidBrush(RGB(0x52,0xaa,0x99));
		//FillRect(hdc, &rc, br);
		DeleteObject(br);
		BitBlt(hdc, 0,0, BackBufferWidth, BackBufferHeight, BackBufferDc, 0, 0, SRCCOPY);
	}
   
	if(!wParam) EndPaint(&ps);
	bHandled = true;
	return 0;
}

LRESULT CMyImage::OnEraseBkg(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
	bHandled = true;
	return TRUE;
}
	  


bool CMyImage::LoadImage(LPCTSTR FileName, Image *img, int ResourceID, bool Bmp, COLORREF transp)
{
	RECT rc;
	GetClientRect(&rc);
	if(BackBufferDc) DeleteObject(BackBufferDc);
	if(BackBufferBm) DeleteObject(BackBufferBm);
	Graphics g(m_hWnd,true);
   
	BackBufferWidth = rc.right;
	BackBufferHeight = rc.bottom;
	float width, height, imgwidth, imgheight, newwidth, newheight;
   	width = rc.right ;
	height = rc.bottom ;

	if(!ResourceID)
	{
		width -=  2;
		height -=  2;
	}

	
	HDC dc = GetDC();

	BackBufferDc = ::CreateCompatibleDC(dc);
	BackBufferBm = ::CreateCompatibleBitmap(dc, BackBufferWidth, BackBufferHeight);
	
	::SelectObject(BackBufferDc, BackBufferBm);


	Graphics gr(BackBufferDc);
	
	Image *bm = NULL;
	bool WhiteBg=false;
	if(img) 
		bm = img;
	else if(FileName)
		bm=new Image(FileName);
	else if(ResourceID) 
	{
		if(!Bmp)
		{
			bm=BitmapFromResource(GetModuleHandle(0), MAKEINTRESOURCE(ResourceID),_T("PNG"));
			WhiteBg=true;
		}
		else 
		{
			bm = new Bitmap(GetModuleHandle(0),MAKEINTRESOURCE(ResourceID));
		}
	}

	if(bm){
	
		imgwidth=bm->GetWidth();
		imgheight=bm->GetHeight();
		ImageWidth = imgwidth;
		ImageHeight = imgheight;
		if(imgwidth/imgheight>width/height)
		{
			if(imgwidth<=width)
			{
				newwidth=imgwidth;
				newheight=imgheight;
			}
			else
			{
			newwidth=width;
			newheight=newwidth/imgwidth*imgheight;
			}
		}
		else
		{
			if(imgheight<=height)
			{
				newwidth=imgwidth;
				newheight=imgheight;
			}
			else
			{
			newheight=height;
			newwidth=(newheight/imgheight)*imgwidth;
			}
		}
	}

	if(ResourceID) {newwidth=bm->GetWidth();newheight=bm->GetHeight();}

	if(WhiteBg)
		gr.Clear(Color(GetRValue(transp),GetGValue(transp),GetBValue(transp)));

	else gr.Clear(Color(255,145,145,145));

	RectF bounds(1, 1, float(width), float(height));
	if((bm) && !bm->GetWidth() && (FileName||ResourceID) )
	{
		LinearGradientBrush 
		br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210), LinearGradientModeBackwardDiagonal); 
		gr.FillRectangle(&br,(float)1, (float)1, (float)width, (float)height);

		LinearGradientBrush 
			brush(bounds, Color(255, 95, 95, 95), Color(255, 125, 125, 125), 
				LinearGradientModeBackwardDiagonal); 

		 StringFormat format;
		format.SetAlignment(StringAlignmentCenter);
		format.SetLineAlignment(StringAlignmentCenter);
		Font font(L"Arial", 12, FontStyleBold);

  		gr.DrawString(TR("Невозможно загрузить изображение"), -1, &font, bounds, &format, &brush);}

		else {
			SolidBrush bb(Color(255, 255, 255, 255));

			LinearGradientBrush 
				br(bounds, Color(255, 255, 255, 255), Color(255, 210, 210, 210), 
				LinearGradientModeBackwardDiagonal); 

		if(!WhiteBg)
			gr.FillRectangle(&br,(float)1, (float)1, (float)width, (float)height);

		IsImage = true;
		//if(FileName || ResourceID){ //return 0;		
		if(bm)
		gr.DrawImage(bm, (int)((ResourceID)?0:1+(width-newwidth)/2), (int)((ResourceID)?0:1+(height-newheight)/2), (int)newwidth,(int)newheight);
		
		}
	
	if(bm && bm!=img)
		delete bm;
	
	ReleaseDC(dc);
	
	IsImage = true;
	Invalidate();
	return false;
}

LRESULT CMyImage::OnLButtonDown(UINT Flags, CPoint Pt)
{
	if(HideParent) ::ShowWindow(GetParent(), SW_HIDE);
	return 0;
}

LRESULT CMyImage::OnKeyDown(TCHAR vk, UINT cRepeat, UINT flags)
{
	if(HideParent) ::ShowWindow(GetParent(), SW_HIDE);
	return 0;
}