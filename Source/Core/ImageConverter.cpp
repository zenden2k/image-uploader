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
#include "ImageConverter.h"
#include "../Common.h"
#include "../LogWindow.h"
#include "Utils/StringUtils.h"
#include "../3rdpart/parser.h"
#include <pcre++.h>
#include <cassert>
#include "../3rdpart/QColorQuantizer.h"

CImageConverter::CImageConverter()
{
	m_generateThumb = false;
	thumbnail_template_ = 0;
   processing_enabled = true;
}

void CImageConverter::setDestinationFolder(const CString &folder)
{
	m_destinationFolder = folder;
}

void CImageConverter::setGenerateThumb(bool generate)
{
	m_generateThumb = generate;
}

CRect CenterRect(CRect r1, CRect intoR2)
{
    r1.OffsetRect(( intoR2.Width()  - r1.Width()) /2,( intoR2.Height()  - r1.Height()) /2);
    return r1;
}

bool CImageConverter::Convert(const CString& sourceFile)
{
	m_sourceFile = sourceFile;
	int fileformat;
	double width,height,imgwidth,imgheight,newwidth,newheight;
	
	Image bm(sourceFile);
	Image* thumbSource = &bm;
	std::auto_ptr<Bitmap> BackBuffer;
	imgwidth = float(bm.GetWidth());
	imgheight = float(bm.GetHeight());
   double NewWidth = _wtof(m_imageConvertingParams.strNewWidth);
   double NewHeight = _wtof(m_imageConvertingParams.strNewHeight);
   if( m_imageConvertingParams.strNewWidth.Right(1)==_T("%"))
   {
      NewWidth = NewWidth*imgwidth/100;
   }

   if( m_imageConvertingParams.strNewHeight.Right(1)==_T("%"))
   {
      NewHeight = NewHeight*imgheight/100;
   }

	width = float(NewWidth);
	height = float(NewHeight);

   if( m_imageConvertingParams.Format < 1 || !processing_enabled ) 
		fileformat = GetSavingFormat(sourceFile);
	else 
		fileformat = m_imageConvertingParams.Format-1;

   if(fileformat == GetSavingFormat(sourceFile) && imgwidth < width && imgheight < height)
   {
      processing_enabled = false;
   }

	newwidth = imgwidth;
	newheight = imgheight;

	// Если включена опция "Оставить без изменений", просто копируем имя исходного файла
	if(!processing_enabled) 
		m_resultFileName = sourceFile;
	else
	{
      if(m_imageConvertingParams.ResizeMode == irmFit)
		{
         if(width && height && ( imgwidth > width || imgheight > height) )
         {
           double S = min(width/imgwidth, height/imgheight);
            newwidth = S * imgwidth;
            newheight = S  * imgheight;
         }
			else if( width && imgwidth > width )
			{
				newwidth = width;
				newheight = newwidth / imgwidth * imgheight;
			}
			else if(height && imgheight > height)
			{
				newheight = height;
				newwidth = newheight/imgheight*imgwidth;
			}
		}
		/*else if(m_imageConvertingParams.ResizeMode == irmCrop)
		{
			if(width>0) newwidth=width;
			if(height>0) newheight=height;
		}*/
		else //if(m_imageConvertingParams.ResizeMode == irmStretch)
		{
			if( imgwidth > width || imgheight > height)
			{
				if(width>0) newwidth=width;
				if(height>0) newheight=height;
			}
			else
			{
				newwidth=imgwidth;
				newheight=imgheight;
			}
		}
		BackBuffer.reset(new Bitmap((int)newwidth,( int)newheight));

		Graphics gr(BackBuffer.get());
		if(fileformat != 2)
			gr.Clear(Color(255,255,255,255));
		else 
			gr.Clear(Color(125,255,255,255));

		//g.SetPageUnit(UnitPixel);
		gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );

		gr.SetPixelOffsetMode(PixelOffsetModeHalf);

ImageAttributes attr;
					attr.SetWrapMode( WrapModeTileFlipXY);
      if(m_imageConvertingParams.ResizeMode == irmFit || m_imageConvertingParams.ResizeMode == irmStretch)
      {
		   
         if((!width && !height) || ((int)newwidth==(int)imgwidth && (int)newheight==(int)imgheight))
			   gr.DrawImage(/*backBuffer*/&bm, (int)0, (int)0, (int)newwidth,(int)newheight);
		   else
            gr.DrawImage(&bm,
                     RectF((int)0, (int)0, (int)newwidth,(int)newheight),
                     0,
                     0,
                    bm.GetWidth(),
                     bm.GetHeight(),
                     UnitPixel,
                     &attr);
			   //gr.DrawImage(/*backBuffer*/&bm, (int)-1, (int)-1, (int)newwidth+2,(int)newheight+2);
      }
      else if(m_imageConvertingParams.ResizeMode == irmCrop)
      {
         int newVisibleWidth = 0;
         int newVisibleHeight = 0;
			double k = 1;
         if(newwidth > newheight)
         {
            newVisibleWidth = min(newwidth, imgwidth);
				k = newVisibleWidth/imgwidth;
            newVisibleHeight = newVisibleWidth/imgwidth*imgheight; 
         }
         else
         {
            newVisibleHeight = min(newheight, imgheight);
				k =newVisibleHeight/imgheight;
            newVisibleWidth = newVisibleHeight/imgheight*imgwidth; 
         }
         CRect r(0,0, newVisibleWidth, newVisibleHeight);
         CRect destRect = CenterRect(r, CRect(0,0,newwidth, newheight));
			CRect croppedRect;
			croppedRect.IntersectRect(CRect(0,0,newwidth, newheight), destRect);
			CRect sourceRect = croppedRect;
			sourceRect.OffsetRect(/*(destRect.left < 0)?*/-destRect.left, /*(destRect.top < 0)?*/-destRect.top);
			sourceRect.left /= k; 
			sourceRect.top /= k; 
			sourceRect.right /= k; 
			sourceRect.bottom /= k; 
			//= sourceRect.MulDiv(1, k);
			gr.DrawImage(&bm,
                     RectF(croppedRect.left, croppedRect.top, croppedRect.Width(), croppedRect.Height())
                     , sourceRect.left, sourceRect.top, sourceRect.Width(), sourceRect.Height(), UnitPixel,
                     &attr);
         /*gr.DrawImage(&bm,
                     RectF((int)0, (int)0, (int)newwidth,(int)newheight),
                     destRect.left, destRect.top, destRect.Width(), destRect.Height(),
                     UnitPixel,
                     &attr);*/
      }
		RectF bounds(0, 0, float(newwidth), float(newheight));

		// Добавляем текст на картинку (если опция включена)
		if(m_imageConvertingParams.AddText)
		{

			SolidBrush brush(Color(GetRValue(m_imageConvertingParams.TextColor),GetGValue(m_imageConvertingParams.TextColor),GetBValue(m_imageConvertingParams.TextColor)));

			int HAlign[6]={0,1,2,0,1,2};	
			int VAlign[6]={0,0,0,2,2,2};	

			m_imageConvertingParams.Font.lfQuality=m_imageConvertingParams.Font.lfQuality|ANTIALIASED_QUALITY ;
			HDC dc = ::GetDC(0);
			Font font(/*L"Tahoma", 10, FontStyleBold*/dc,&m_imageConvertingParams.Font);
			SolidBrush brush2(Color(70,0,0,0));
			RectF bounds2(1, 1, float(newwidth), float(newheight)+1);
			ReleaseDC(0, dc);
			DrawStrokedText(gr, m_imageConvertingParams.Text,bounds2,font,MYRGB(255,m_imageConvertingParams.TextColor),MYRGB(180,m_imageConvertingParams.StrokeColor),HAlign[m_imageConvertingParams.TextPosition],VAlign[m_imageConvertingParams.TextPosition], 1);
		}

		if(m_imageConvertingParams.AddLogo)
		{
			Bitmap logo(m_imageConvertingParams.LogoFileName);
			if(logo.GetLastStatus() == Ok)
			{
				int x,y;
				int logowidth,logoheight;
				logowidth=logo.GetWidth();
				logoheight=logo.GetHeight();
				if(m_imageConvertingParams.LogoPosition<3) y=0;
				else y=int(newheight-logoheight);
				if(m_imageConvertingParams.LogoPosition==0||m_imageConvertingParams.LogoPosition==3)
					x=0;
				if(m_imageConvertingParams.LogoPosition==2||m_imageConvertingParams.LogoPosition==5)
					x=int(newwidth-logowidth);
				if(m_imageConvertingParams.LogoPosition==1||m_imageConvertingParams.LogoPosition==4)
					x=int((newwidth-logowidth)/2);

				gr.DrawImage(&logo, (int)x, (int)y,logowidth,logoheight);
			}
		}
		thumbSource = BackBuffer.get();
		MySaveImage(BackBuffer.get(),GenerateFileName(L"img%md5.jpg",1,CPoint()),m_resultFileName,fileformat,m_imageConvertingParams.Quality);
	} 

	if(!processing_enabled)
	{
		CString Ext = GetFileExt(sourceFile);
		if(Ext == _T("png"))
			fileformat = 1;
		else fileformat = 0;
	}
	if(m_generateThumb)
	{
		// Генерирование превьюшки с шаблоном в отдельной функции
		int thumbFormat = fileformat;
		if(m_thumbCreatingParams.Format == tfJPEG)
			thumbFormat = 0;
		else if(m_thumbCreatingParams.Format == tfPNG)
			thumbFormat = 1;
		else if(m_thumbCreatingParams.Format == tfGIF)
			thumbFormat = 2;
		createThumb(thumbSource, thumbFormat);
	}
	return true;
}

bool CImageConverter::createThumb(Gdiplus::Image *bm, int fileformat)
{
	bool result = false;
	HDC dc = ::GetDC(0);
	int newwidth = bm->GetWidth();
	int newheight = bm->GetHeight();
	zint64 FileSize = IuCoreUtils::getFileSize(WCstringToUtf8((m_sourceFile)));
	//TCHAR SizeBuffer[100]=_T("\0");
	//if(FileSize>0)
	//	NewBytesToString(FileSize,SizeBuffer,sizeof(SizeBuffer));

	//CString ThumbnailText = m_thumbCreatingParams.Text; // Text that will be drawn on thumbnail

	//ThumbnailText.Replace(_T("%width%"), IntToStr(newwidth)); //Replacing variables names with their values
	//ThumbnailText.Replace(_T("%height%"), IntToStr(newheight));
//	ThumbnailText.Replace(_T("%size%"), SizeBuffer);

#if 0
	int thumbwidth=m_thumbCreatingParams.ThumbWidth;
	if(m_thumbCreatingParams.UseThumbTemplate)
	{
		
		Graphics g1(dc);
		Image templ(IU_GetDataFolder()+_T("thumb.png"));
		int ww = templ.GetWidth();
		CString s;


		Font font(dc, &m_thumbCreatingParams.ThumbFont);

		RectF TextRect;

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(_T("test"),-1,&font,PointF(0,0),&TextRect);


		thumbwidth-=4;
		int thumbheight = int((float)thumbwidth/(float)newwidth*newheight);


		int LabelHeight= int(TextRect.Height+1);
		int RealThumbWidth=thumbwidth+4;
		int RealThumbHeight=thumbheight+19;

		Bitmap ThumbBuffer(RealThumbWidth, RealThumbHeight, &g1);
		Graphics thumbgr(&ThumbBuffer);
		thumbgr.SetPageUnit(UnitPixel);
		thumbgr.Clear(Color(255,255,255,255));
		RectF thu((float)(m_thumbCreatingParams.DrawFrame?1:0), (float)(m_thumbCreatingParams.DrawFrame?1:0), (float)thumbwidth,(float)thumbheight);
		thumbgr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );

		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		thumbgr.SetSmoothingMode( SmoothingModeHighQuality);


		thumbgr.SetSmoothingMode(SmoothingModeAntiAlias);
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );

		RectF t((float)0, (float)12, (float)5,(float)RealThumbHeight);

		thumbgr.DrawImage(&templ,t,0,13,4,4,UnitPixel);

		RectF t2((float)m_thumbCreatingParams.ThumbWidth-6, (float)9, (float)6,(float)RealThumbHeight);

		thumbgr.DrawImage(&templ,t2,158,11,6,6,UnitPixel);

		RectF t3((float)6, (float)0, (float)RealThumbWidth-8,(float)6);

		thumbgr.DrawImage(&templ,t3,12,0,7,5,UnitPixel);


		RectF t4((float)0, (float)RealThumbHeight-17, (float)RealThumbWidth,(float)17);

		thumbgr.DrawImage(&templ,t4,71.0,92,4,19,UnitPixel);

		thumbgr.DrawImage(&templ,0.0,0.0,0,0,29,29,UnitPixel);
		thumbgr.DrawImage(&templ,m_thumbCreatingParams.ThumbWidth-6,0.0,164-6,0.0,6,9,UnitPixel);
		thumbgr.DrawImage(&templ,0.0,RealThumbHeight-17,0.0,94.0,70,17,UnitPixel);
		thumbgr.DrawImage(&templ,RealThumbWidth-29,RealThumbHeight-29,135.0,82,29,29,UnitPixel);

		SolidBrush whiteBr(Color(255,255,255));
		thumbgr.FillRectangle(&whiteBr, 2,2,thumbwidth,thumbheight);
		thumbgr.DrawImage(bm,(float)2.0/*item->DrawFrame?1:0-1*/,(float)2.0/*(int)item->DrawFrame?1:0-1*/,(float)thumbwidth,(float)thumbheight);

		thumbgr.SetPixelOffsetMode(PixelOffsetModeHalf);

		if(m_thumbCreatingParams.ThumbAddImageSize) // If we need to draw text on thumbnail
		{
			thumbgr.SetPixelOffsetMode(PixelOffsetModeDefault );
			SolidBrush   br222(MYRGB(179,RGB(255,255,255)));
			RectF TextBounds((float)65, (float)RealThumbHeight-17, (float)RealThumbWidth-65-11,(float)17);

			DrawStrokedText(thumbgr,/* Buffer*/ ThumbnailText,TextBounds,font,MYRGB(179,RGB(255,255,255))/*MYRGB(255,params.ThumbTextColor)*/,MYRGB(90,RGB(0,0,0)/*params.StrokeColor*/),1,1, 1);

		}

		Pen p(MYRGB(255,m_thumbCreatingParams.FrameColor));

		if(m_thumbCreatingParams.ThumbAddImageSize){
			StringFormat format;
			format.SetAlignment(StringAlignmentCenter);
			format.SetLineAlignment(StringAlignmentCenter);
			// Font font(L"Tahoma", 7, FontStyleBold);
			SolidBrush LabelBackground(Color(255,140,140,140));;

			int LabelAlpha=(m_thumbCreatingParams.TextOverThumb)?m_thumbCreatingParams.ThumbAlpha:255;
			RectF TextBounds((float)1, float(RealThumbHeight-LabelHeight), float(RealThumbWidth-1), float(LabelHeight+1));
		}

		if(fileformat == 2) 
			fileformat = 0;
		
		result = MySaveImage(&ThumbBuffer,_T("thumb"),m_thumbFileName,fileformat,93);
	}
	else
	{
		// Old styled thumb
		Graphics g1(dc);
		Font font(dc, &m_thumbCreatingParams.ThumbFont);

		RectF TextRect;

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(_T("test"),-1,&font,PointF(0,0),&TextRect);

		if(m_thumbCreatingParams.DrawFrame)
			thumbwidth -= 2;
		int thumbheight = int((float)thumbwidth/(float)newwidth*newheight);

		int LabelHeight= int(TextRect.Height+1);
		int RealThumbWidth=thumbwidth+(m_thumbCreatingParams.DrawFrame?2:0);
		int RealThumbHeight=(m_thumbCreatingParams.DrawFrame?2:0)+thumbheight+((m_thumbCreatingParams.ThumbAddImageSize&&(!m_thumbCreatingParams.TextOverThumb))?LabelHeight:0);

		Bitmap ThumbBuffer(RealThumbWidth, RealThumbHeight, &g1);
		Graphics thumbgr(&ThumbBuffer);
		thumbgr.SetPageUnit(UnitPixel);
		RectF thu((float)(m_thumbCreatingParams.DrawFrame?1:0), (float)(m_thumbCreatingParams.DrawFrame?1:0), (float)thumbwidth,(float)thumbheight);
		thumbgr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		thumbgr.SetSmoothingMode( SmoothingModeHighQuality);
		thumbgr.SetSmoothingMode(SmoothingModeAntiAlias);
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		thumbgr.DrawImage(/*backBuffer*/bm,(float)-0.5f/*item->DrawFrame?1:0-1*/,(float)-0.5/*(int)item->DrawFrame?1:0-1*/,(float)RealThumbWidth+1,(float)thumbheight+1.5);
		thumbgr.SetPixelOffsetMode(PixelOffsetModeHalf);
		Pen p(MYRGB(255,m_thumbCreatingParams.FrameColor));

		if(m_thumbCreatingParams.ThumbAddImageSize)
		{
			StringFormat format;
			format.SetAlignment(StringAlignmentCenter);
			format.SetLineAlignment(StringAlignmentCenter);

			SolidBrush LabelBackground(Color(255,140,140,140));;
			int LabelAlpha=(m_thumbCreatingParams.TextOverThumb)?m_thumbCreatingParams.ThumbAlpha:255;
			DrawGradient(thumbgr,Rect(0,RealThumbHeight-LabelHeight-(m_thumbCreatingParams.DrawFrame?1:0),RealThumbWidth,LabelHeight),MYRGB(LabelAlpha,m_thumbCreatingParams.ThumbColor1), MYRGB(LabelAlpha,m_thumbCreatingParams.ThumbColor2));
			RectF TextBounds(1.0,(float)RealThumbHeight-LabelHeight, (float)RealThumbWidth-1, float(LabelHeight+1));

			thumbgr.SetPixelOffsetMode(PixelOffsetModeDefault );
			SolidBrush   br222(MYRGB(255,m_thumbCreatingParams.ThumbTextColor));
			DrawStrokedText(thumbgr, /*Buffer*/ThumbnailText, TextBounds, font, MYRGB(255,m_thumbCreatingParams.ThumbTextColor), MYRGB(90,0,0,0/*params.StrokeColor*/),1,1, 1);
		}

		thumbgr.SetPixelOffsetMode(   PixelOffsetModeHalf);
		thumbgr.SetSmoothingMode(SmoothingModeDefault);
		p.SetAlignment(PenAlignmentInset);

		if(m_thumbCreatingParams.DrawFrame)
			DrawRect(ThumbBuffer,MYRGB(255,m_thumbCreatingParams.FrameColor),Rect(0,0,RealThumbWidth,RealThumbHeight));
#endif
		/*if(fileformat == 2) 
			fileformat = 0;*/

		// Saving thumbnail (without template)
		Image *res = 0;
		if(createThumbnail(bm, &res, FileSize, fileformat))
		{
			result = MySaveImage(res,_T("thumb"),m_thumbFileName,fileformat,m_thumbCreatingParams.Quality);
			delete res;
		}
	//}
	ReleaseDC(0, dc);
	return result;
}

// hack for stupid GDIplus
void changeAplhaChannel(Bitmap& source, Bitmap& dest, int sourceChannel, int destChannel )
{
	Rect r( 0, 0, source.GetWidth(),source.GetHeight() );
	BitmapData  bdSrc;
	BitmapData bdDst;
	source.LockBits( &r,  ImageLockModeRead , PixelFormat32bppARGB,&bdSrc);
	dest.LockBits( &r,  ImageLockModeWrite   , PixelFormat32bppARGB , &bdDst);

	BYTE* bpSrc = (BYTE*)bdSrc.Scan0;
	BYTE* bpDst = (BYTE*)bdDst.Scan0;
	bpSrc += (int)sourceChannel;
	bpDst += (int)destChannel;

	for ( int i = r.Height * r.Width; i > 0; i-- )
	{
		//if(*bpSrc != 255)
		{
			*bpDst = (float(255-*bpSrc)/255) *  *bpDst;
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
	source.UnlockBits( &bdSrc );
	dest.UnlockBits( &bdDst );
}

bool CImageConverter::createThumbnail(Gdiplus::Image *image, Gdiplus::Image ** outResult, zint64 fileSize, int fileformat)
{
	assert(thumbnail_template_);
	bool result = false;
	const ThumbnailData* data = thumbnail_template_->getData();
	CDC dc = ::GetDC(0);
	int newwidth = image->GetWidth();
	int newheight = image->GetHeight();
	int FileSize = MyGetFileSize(m_sourceFile);
	/*TCHAR SizeBuffer[100]=_T("\0");
	if(FileSize>0)
		NewBytesToString(FileSize,SizeBuffer,sizeof(SizeBuffer));*/

	CString sizeString = Utf8ToWCstring(IuCoreUtils::fileSizeToString(fileSize));
	CString ThumbnailText = m_thumbCreatingParams.Text; // Text that will be drawn on thumbnail

	ThumbnailText.Replace(_T("%width%"), IntToStr(newwidth)); //Replacing variables names with their values
	ThumbnailText.Replace(_T("%height%"), IntToStr(newheight));
	ThumbnailText.Replace(_T("%size%"), sizeString);

	int thumbwidth=m_thumbCreatingParams.ThumbWidth;
	int thumbheight =m_thumbCreatingParams.ThumbHeight;
	Graphics g1(dc);
	CString filePath = Utf8ToWCstring(thumbnail_template_->getSpriteFileName());
	Image templ(filePath);
	int ww = templ.GetWidth();
	CString s;

		
		LOGFONT lf;
		StringToFont(_T("Tahoma,7,b,204"), &lf);
		if(thumbnail_template_->existsParam("Font"))
		{
			std::string font = thumbnail_template_->getParamString("Font");
			CString wide_text = Utf8ToWCstring(font);
			StringToFont(wide_text, &lf);
		}
		Font font(dc, &lf);
		RectF TextRect;

		/*StringFormat * f = StringFormat::GenericTypographic()->Clone();
	f->SetTrimming(StringTrimmingNone);
	f->SetFormatFlags( StringFormatFlagsMeasureTrailingSpaces);*/

StringFormat format;
		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitWorld);
		g1.MeasureString(_T("test"),-1,&font,PointF(0,0),&format, &TextRect);
//		delete f;
		m_Vars["TextWidth"] = IuCoreUtils::toString((int)TextRect.Width);
		m_Vars["TextHeight"] =  IuCoreUtils::toString((int)TextRect.Height);
		m_Vars["UserText"] =  WCstringToUtf8(ThumbnailText);
		std::string textTempl = thumbnail_template_->getParamString("Text");
		if (textTempl.empty())
			textTempl =  "$(UserText)";
		CString textToDraw = Utf8ToWCstring(ReplaceVars(textTempl));

		for(std::map<std::string, std::string >::const_iterator it = data->colors_.begin(); it!=data->colors_.end(); ++it)
		{
			m_Vars[it->first] = it->second;	
		}

		m_Vars["DrawText"]= IuCoreUtils::toString(m_Vars["DrawText"]=="1" && m_thumbCreatingParams.ThumbAddImageSize);

      if(!m_thumbCreatingParams.ScaleByHeight)
      {
		   thumbwidth -= EvaluateExpression(thumbnail_template_->getWidthAddition());
         if(thumbwidth<10) thumbwidth = 10;
		    thumbheight = int((float)thumbwidth/(float)newwidth*newheight);
      }
      else
      {
         
         thumbheight -= EvaluateExpression(thumbnail_template_->getHeightAddition());
         if(thumbheight<10) thumbheight = 10;
		    thumbwidth = int((float)thumbheight/(float)newheight*newwidth);
      }


		int LabelHeight = int(TextRect.Height+1);
		int RealThumbWidth = thumbwidth + EvaluateExpression(thumbnail_template_->getWidthAddition());
		int RealThumbHeight = thumbheight + EvaluateExpression(thumbnail_template_->getHeightAddition());
	
			m_Vars["Width"] = IuCoreUtils::toString(RealThumbWidth);
		m_Vars["Height"] = IuCoreUtils::toString(RealThumbHeight);
		Bitmap *ThumbBuffer = new Bitmap(RealThumbWidth, RealThumbHeight, &g1);
		Graphics thumbgr(ThumbBuffer);
		thumbgr.SetPageUnit(UnitPixel);

      if(fileformat == 0 || fileformat == 2)
        thumbgr.Clear(MYRGB(255,m_thumbCreatingParams.BackgroundColor));
	//thumbgr.Clear(Color(255,255,255,255));
		RectF thu((float)(m_thumbCreatingParams.DrawFrame?1:0), (float)(m_thumbCreatingParams.DrawFrame?1:0), (float)thumbwidth,(float)thumbheight);
		thumbgr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );
		//thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		//thumbgr.SetSmoothingMode( SmoothingModeHighQuality);
	////	thumbgr.SetSmoothingMode(SmoothingModeAntiAlias);
	thumbgr.SetSmoothingMode(SmoothingModeNone);
		//thumbgr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
		//thumbgr.SetCompositingQuality(CompositingQualityHighQuality);
	
		Bitmap *MaskBuffer = new Bitmap(RealThumbWidth, RealThumbHeight, PixelFormat32bppARGB);
		Graphics maskgr(MaskBuffer);
 
		for(size_t i = 0; i< data->drawing_operations_.size(); i++)
		{
			bool performOperation = true;
			if( !data->drawing_operations_[i].condition.empty())
			{
				performOperation = EvaluateExpression(data->drawing_operations_[i].condition)!=0;
			}
			if(!performOperation) continue;
			Graphics *gr = &thumbgr;
			std::string type = data->drawing_operations_[i].type;
			CRect rc;
			EvaluateRect(data->drawing_operations_[i].rect, &rc);
			if(data->drawing_operations_[i].destination == "mask")
			{
				gr = &maskgr;
			}
			if(type == "text")
			{
				std::string colorsStr = data->drawing_operations_[i].text_colors;
				std::vector<std::string> tokens;
				IuStringUtils::Split(colorsStr, " ", tokens, 2);
				unsigned int color1 = 0xB3FFFFFF ;
				unsigned int strokeColor = 0x5A000000;
				if(tokens.size()> 0)
					color1 = 	EvaluateColor(tokens[0]);
				if(tokens.size()> 1)
					strokeColor = EvaluateColor(tokens[1]);
				RectF TextBounds((float)rc.left, (float)rc.top, (float)rc.right,(float)rc.bottom);
				thumbgr.SetPixelOffsetMode(PixelOffsetModeDefault );
				DrawStrokedText(*gr,/* Buffer*/ textToDraw,TextBounds,font,Color(color1),Color(strokeColor) /*params.StrokeColor*/,1,1, 1);
				thumbgr.SetPixelOffsetMode(PixelOffsetModeNone );
				
			}
			else if(type == "fillrect")
			{
				Brush * fillBr = CreateBrushFromString(data->drawing_operations_[i].brush, rc);
				gr->FillRectangle(fillBr, rc.left, rc.top, rc.right,rc.bottom);
				delete fillBr;
			}
			else if(type == "drawrect")
			{
				GraphicsPath path;
				//path.AddRectangle(
				std::vector<std::string> tokens;
				IuStringUtils::Split(data->drawing_operations_[i].pen, " ", tokens, 2);
				if(tokens.size() > 1)
				{

					unsigned int color = EvaluateColor(tokens[0]);
					int size = EvaluateExpression( tokens[1]);
					Gdiplus::Pen  p(color, size);
					if(size == 1)
					{
						rc.right --;
						rc.bottom --;
					}
					p.SetAlignment( PenAlignmentInset );
					gr->DrawRectangle(&p, rc.left, rc.top, rc.right,rc.bottom);
				}
			}
			else if(type == "blt")
			{
				RECT sourceRect;
				EvaluateRect(data->drawing_operations_[i].source_rect, &sourceRect);
				Rect t((float)rc.left, (float)rc.top, (float)rc.right,(float)rc.bottom);
				
				
				if(data->drawing_operations_[i].source == "image")
				{
					SolidBrush whiteBr(Color(160,130,100));
					//thumbgr.FillRectangle(&whiteBr, (int)t.X, (int) t.Y/*(int)item->DrawFrame?1:0-1*/,thumbwidth,thumbheight);
					//thumbgr.SetInterpolationMode(InterpolationModeNearestNeighbor  );
					ImageAttributes attr;
					attr.SetWrapMode( WrapModeTileFlipXY);
					Rect dest(t.X, t.Y, thumbwidth, thumbheight);
					
					Bitmap tempImage(RealThumbWidth, RealThumbHeight, PixelFormat32bppARGB);
					Graphics tempGr(&tempImage);
					tempGr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );
					tempGr.SetSmoothingMode(SmoothingModeHighQuality);
					tempGr.SetPixelOffsetMode(PixelOffsetModeHighQuality );
					tempGr.DrawImage(image,dest,(INT)0,(int)0,(int)image->GetWidth(),(int)image->GetHeight(),UnitPixel, &attr);
					changeAplhaChannel(*MaskBuffer, tempImage, 3,3); 
					gr->DrawImage(&tempImage, 0,0);
					tempGr.SetSmoothingMode(SmoothingModeNone);
				}	
				else
				{
					ImageAttributes attr;
				   attr.SetWrapMode(WrapModeClamp); // we need this to avoid some glitches on the edges of interpolated image
               thumbgr.SetInterpolationMode(InterpolationModeHighQualityBicubic  );
               thumbgr.SetSmoothingMode(SmoothingModeHighQuality);
					gr->DrawImage(&templ,t,(int)sourceRect.left, (int)sourceRect.top, (int)sourceRect.right, (int)sourceRect.bottom,UnitPixel,&attr);
               thumbgr.SetSmoothingMode(SmoothingModeNone);
            }
			}
		}


		delete MaskBuffer;
		*outResult = ThumbBuffer;
	
	return true;
}

void CImageConverter::setImageConvertingParams(const ImageConvertingParams &params)
{
	m_imageConvertingParams = params;
}

void CImageConverter::setThumbCreatingParams(const ThumbCreatingParams &params)
{
	m_thumbCreatingParams = params;
}

bool MySaveImage(Image *img, const CString& szFilename, CString& szBuffer, int Format,int Quality,LPCTSTR Folder)
{
	if(Format==-1) Format=0;
	std::auto_ptr<Bitmap> quantizedImage;
	TCHAR szImgTypes[3][4]={_T("jpg"),_T("png"),_T("gif")};
	TCHAR szMimeTypes[3][12]={_T("image/jpeg"),_T("image/png"),_T("image/gif")};
	CString szNameBuffer;
	TCHAR szBuffer2[MAX_PATH];
	if(IsImage(szFilename) )
	szNameBuffer = GetOnlyFileName(szFilename);
	else
	szNameBuffer = szFilename;
	CString userFolder ;
	if(Folder)
		userFolder= Folder;
	if(userFolder.Right(1)!=_T('\\')) userFolder += _T('\\');
	wsprintf(szBuffer2,_T("%s%s.%s"),(LPCTSTR)Folder?userFolder:IUTempFolder,(LPCTSTR)szNameBuffer,/*(int)GetTickCount(),*/szImgTypes[Format]);
	CString resultFilename=GetUniqFileName(szBuffer2);
	IU_CreateFilePath(resultFilename);
	CLSID clsidEncoder;
	EncoderParameters eps;
	eps.Count = 1;

	if(Format == 0) //JPEG
	{
		eps.Parameter[0].Guid = EncoderQuality;
		eps.Parameter[0].Type = EncoderParameterValueTypeLong;
		eps.Parameter[0].NumberOfValues = 1;
		eps.Parameter[0].Value = &Quality;
	}
	else if (Format == 1) //PNG
	{
		eps.Parameter[0].Guid = EncoderCompression;
		eps.Parameter[0].Type = EncoderParameterValueTypeLong;
		eps.Parameter[0].NumberOfValues = 1;
		eps.Parameter[0].Value = &Quality;
	}
	else if(Format == 2)
	{
		QColorQuantizer quantizer;
		quantizedImage.reset ( quantizer.GetQuantized(img, QColorQuantizer::Octree, (Quality<50)?16:256) );
		if(quantizedImage.get() !=0)
			img = quantizedImage.get();
	}

	Gdiplus::Status result;

	if(GetEncoderClsid(szMimeTypes[Format], &clsidEncoder) != -1)
	{
		if(Format == 0)
			result = img->Save(resultFilename, &clsidEncoder, &eps);
		else
			result =img->Save(resultFilename, &clsidEncoder);
	}
	else 
		return false;
	if(result != Ok)
	{
		WriteLog(logError, _T("Image Converter"), _T("Could not save image at path \r\n")+resultFilename);
		return false;
	}
	szBuffer = resultFilename;
	return true;
}


void DrawGradient(Graphics &gr,Rect rect,Color &Color1,Color &Color2)
{
	Bitmap bm(rect.Width,rect.Height,&gr);
	Graphics temp(&bm);
	LinearGradientBrush 
		brush(/*TextBounds*/Rect(0,0,rect.Width,rect.Height), Color1, Color2,LinearGradientModeVertical);

	temp.FillRectangle(&brush,Rect(0,0,rect.Width,rect.Height));
	gr.DrawImage(&bm, rect.X,rect.Y);
}


void DrawRect(Bitmap &gr,Color &color,Rect rect)
{
	int i;
	SolidBrush br(color);
	for(i=rect.X;i<rect.Width;i++)
	{
		gr.SetPixel(i,0,color);
		gr.SetPixel(i,rect.Height-1,color);
	}

	for(i=rect.Y;i<rect.Height;i++)
	{
		gr.SetPixel(0,i,color);
		gr.SetPixel(rect.Width-1,i,color);
	}
}

const CString CImageConverter::getThumbFileName()
{
	return m_thumbFileName;
}

const CString CImageConverter::getImageFileName()
{
	return m_resultFileName;
}

Rect MeasureDisplayString(Graphics& graphics, CString text,RectF boundingRect,
                                            Font& font)
{
    CharacterRange charRange(0, text.GetLength());
	 Region pCharRangeRegions;
	 StringFormat strFormat;
	  strFormat.SetMeasurableCharacterRanges(1, &charRange);
	/*System.Drawing.CharacterRange[] ranges  = 
                                       { new System.Drawing.CharacterRange(0, 
                                                               text.Length) };*/
   /// System.Drawing.Region[]         regions = new System.Drawing.Region[1];

 //   strFormat.SetMeasurableCharacterRanges (pCharRangeRegions);

    graphics.MeasureCharacterRanges (text, text.GetLength(), &font, boundingRect, &strFormat, 1, &pCharRangeRegions);
	Rect rc;
     pCharRangeRegions.GetBounds (&rc, &graphics);

    return rc;
}
/*
void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Font &font,Color &ColorText,Color &ColorStroke,int HorPos,int VertPos, int width)
{
	RectF OriginalTextRect;
	//	, NewTextRect;
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
//	GraphicsPath path;
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
//	gr.SolidBrush br3(Color(0,255,0));
	
	//gr.FillRectangle(&br2, 0,0,(int)(realTextBounds.Width+realTextBounds.X),(int)(realTextBounds.Height+realTextBounds.Y));
	 
	gr.DrawImage(&temp, destRect,(int)realTextBounds.X, (int)realTextBounds.Y,(int)(realTextBounds.Width),(int)(realTextBounds.Height), UnitPixel);
}*/

void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Font &font,Color &ColorText,Color &ColorStroke,int HorPos,int VertPos, int width)
{
        RectF OriginalTextRect, NewTextRect;
        FontFamily ff;
        font.GetFamily(&ff);
        gr.SetPageUnit(UnitPixel);
        gr.MeasureString(Text,-1,&font,PointF(0,0),&OriginalTextRect);


        Font NewFont(&ff,48,font.GetStyle(),UnitPixel);
        gr.MeasureString(Text,-1,&NewFont,RectF(0,0,5000,1600),&NewTextRect);
        OriginalTextRect.Height = OriginalTextRect.Height-OriginalTextRect.Y;
        float newwidth,newheight;
        newheight = OriginalTextRect.Height;
        newwidth=OriginalTextRect.Height/NewTextRect.Height*NewTextRect.Width;
        float k = 2*width*NewTextRect.Height/OriginalTextRect.Height;
        SolidBrush br(ColorText);
        Bitmap temp((int)NewTextRect.Width,(int)NewTextRect.Height,&gr);


        Graphics gr_temp(&temp);
        StringFormat format;
        gr_temp.SetPageUnit(UnitPixel);
        GraphicsPath path;
        gr_temp.SetSmoothingMode( SmoothingModeHighQuality);
        path.AddString(Text, -1,&ff, (int)NewFont.GetStyle(), NewFont.GetSize(), Point(0,0), &format);


        Pen pen(ColorStroke,(float)k);
        pen.SetAlignment(PenAlignmentCenter);


        float x,y;
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


        gr.DrawImage(&temp,(int)(Bounds.GetLeft()+x),(int)(Bounds.GetTop()+y),(int)(newwidth),(int)(newheight));
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
void CImageConverter::setThumbnail(Thumbnail * thumb)
{
	thumbnail_template_ = thumb;
}

bool CImageConverter::EvaluateRect(const std::string& rectStr,  RECT * out)
{
	std::vector<std::string> values;
	IuStringUtils::Split(rectStr, ";", values, 4);
	if(values.size() != 4) return false;
	int* target = reinterpret_cast<int*>(out);
	for(size_t i=0; i< values.size(); i++)
	{
		int coord = EvaluateExpression(IuStringUtils::Trim(values[i]));
		*target = coord;
		target++;
	}
	return true;
}

int CImageConverter::EvaluateExpression(const std::string& expr)
{
	std::string processedExpr = ReplaceVars(expr);
	return EvaluateSimpleExpression(processedExpr);
}


zint64 CImageConverter::EvaluateSimpleExpression(const std::string& expr) const
{	
	TParser parser;zint64 res = 0;
	try
	{
		parser.Compile(expr.c_str());
		res = (parser.Evaluate());
	}
	catch(...)
	{
	}
	return res;
}

std::string  CImageConverter::ReplaceVars(const std::string& expr)
{
	std::string  Result =  expr;

	pcrepp::Pcre reg("\\$\\(([A-z0-9_]*?)\\)", "imc");
	std::string str = (expr);
	size_t pos = 0;
	while (pos < str.length()) 
	{
		if( reg.search(str, pos)) 
		{
			pos = reg.get_match_end()+1;
			std::string vv = reg[0];
			std::string value = m_Vars[vv];
			
			Result = IuStringUtils::Replace(Result,std::string("$(") + vv + std::string(")"), value);
		}
		else
			break;
	}

	//Result  = IuStringUtils::Replace(Result,std::string("#"), "0x");
	{
		pcrepp::Pcre reg("\\#([0-9A-Fa-f]+)", "imc");
	std::string str = (Result);
	size_t pos = 0;
	while (pos < str.length()) 
	{
		if( reg.search(str, pos)) 
		{
			pos = reg.get_match_end()+1;
			std::string vv = reg[0];
			unsigned int res = strtoul(vv.c_str(),0,16);
			std::string value = m_Vars[vv];
			
			Result = IuStringUtils::Replace(Result,std::string("#") + vv , IuCoreUtils::toString(res));
		}
		else
			break;
	}
	}
	return Result;
}

Gdiplus::Brush * CImageConverter::CreateBrushFromString(const std::string& brStr, RECT rect)
{
	std::vector<std::string> tokens; 
		IuStringUtils::Split(brStr, ":", tokens, 10);
		if(tokens[0] == "solid")
		{
			unsigned int color = 0;
			if(tokens.size() >= 2)
			{
				 color = EvaluateColor(tokens[1]) ;
			}
			SolidBrush* br = new SolidBrush(Color(color));
			return br;
		}
		else if(tokens[0] == "gradient" && tokens.size()>1)
		{
			std::vector<std::string> params; 
			IuStringUtils::Split(tokens[1], " ", params, 10);
			if(params.size() >= 3)
			{
				unsigned int color1 = EvaluateColor(params[0]);
				unsigned int color2 = EvaluateColor(params[1]);
				int type = -1;
				std::string gradType = params[2];
				if(gradType == "vert")
				{
					type = LinearGradientModeVertical;
				}
				else if(gradType == "hor")
				{	
					type = LinearGradientModeHorizontal;
				}
				else if(gradType == "diag1")
				{	
					type = LinearGradientModeForwardDiagonal;
				}
				else if(gradType == "diag2")
				{	
					type = LinearGradientModeBackwardDiagonal;
				}
				//return new LinearGradientBrush(Rect(0,0, /*rect.left+*/rect.right , /*rect.top+*/rect.bottom ), Color(color1), Color(color2), LinearGradientMode(type));
				return new LinearGradientBrush(RectF(rect.left,-0.5+rect.top, /*rect.left+*/rect.right , /*rect.top+*/rect.bottom ), Color(color1), Color(color2), LinearGradientMode(type));
			}
		}
		SolidBrush*   br = new SolidBrush(0);
		return br;
}

unsigned int CImageConverter::EvaluateColor(const std::string& expr)
{
	unsigned int color  = 0;
	color = EvaluateExpression(expr);
	color = ((color<<8)>>8) | ((255-(color>>24))<<24);
	return color;
}

 void  CImageConverter::setEnableProcessing(bool enable)
 {
    processing_enabled = enable;
 }