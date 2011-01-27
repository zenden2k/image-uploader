#include "ImageConverter.h"
#include "../Common.h"
#include "../LogWindow.h"
CImageConverter::CImageConverter()
{
	m_generateThumb = false;
}

void CImageConverter::setDestinationFolder(const CString &folder)
{
	m_destinationFolder = folder;
}

void CImageConverter::setGenerateThumb(bool generate)
{
	m_generateThumb = generate;
}

bool CImageConverter::Convert(const CString& sourceFile)
{
	m_sourceFile = sourceFile;
	int fileformat;

	if( m_imageConvertingParams.Format < 1 ) 
		fileformat = GetSavingFormat(sourceFile);
	else 
		fileformat = m_imageConvertingParams.Format-1;

	float width,height,imgwidth,imgheight,newwidth,newheight;

	Image bm(sourceFile);
	imgwidth = bm.GetWidth();
	imgheight = bm.GetHeight();

	width = m_imageConvertingParams.NewWidth;
	height = m_imageConvertingParams.NewHeight;

	newwidth=imgwidth;
	newheight=imgheight;


	// Если включена опция "Оставить без изменений", просто копируем имя исходного файла
	if(m_imageConvertingParams.KeepAsIs) 
		m_resultFileName= sourceFile;

	else
	{
		if(m_imageConvertingParams.SaveProportions)
		{
			if( width && imgwidth > width )
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
		else
		{
			if(width>0) newwidth=width;
			if(height>0) newheight=height;
		}

		/*Graphics g(m_hWnd, true);
		g.SetPageUnit(UnitPixel);
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);*/
		Bitmap BackBuffer(newwidth, newheight/*, &g*/);


		Graphics gr(&BackBuffer);
		if(fileformat != 2)
			gr.Clear(Color(255,255,255,255));
		else 
			gr.Clear(Color(125,255,255,255));

		//g.SetPageUnit(UnitPixel);
		gr.SetInterpolationMode(InterpolationModeHighQualityBicubic );

		gr.SetPixelOffsetMode(PixelOffsetModeHalf);
		if((!width && !height) || ((int)newwidth==(int)imgwidth && (int)newheight==(int)imgheight))
			gr.DrawImage(/*backBuffer*/&bm, (int)0, (int)0, (int)newwidth,(int)newheight);
		else
			gr.DrawImage(/*backBuffer*/&bm, (int)-1, (int)-1, (int)newwidth+2,(int)newheight+2);

		RectF bounds(0, 0, float(newwidth), float(newheight));

		// Добавляем текст на картинку (если опция включена)
		if(m_imageConvertingParams.AddText)
		{

			SolidBrush brush(Color(GetRValue(m_imageConvertingParams.TextColor),GetGValue(m_imageConvertingParams.TextColor),GetBValue(m_imageConvertingParams.TextColor)));

			int HAlign[6]={0,1,2,0,1,2};	
			int VAlign[6]={0,0,0,2,2,2};	

			m_imageConvertingParams.Font.lfQuality=m_imageConvertingParams.Font.lfQuality|ANTIALIASED_QUALITY ;
			Font font(/*L"Tahoma", 10, FontStyleBold*/::GetDC(0),&m_imageConvertingParams.Font);


		   
			SolidBrush brush2(Color(70,0,0,0));

			RectF bounds2(1, 1, float(newwidth), float(newheight)+1);
			DrawStrokedText(gr, m_imageConvertingParams.Text,bounds2,font,MYRGB(255,m_imageConvertingParams.TextColor),MYRGB(180,m_imageConvertingParams.StrokeColor),HAlign[m_imageConvertingParams.TextPosition],VAlign[m_imageConvertingParams.TextPosition], 1);
		}

		if(m_imageConvertingParams.AddLogo)
		{

			Bitmap logo(m_imageConvertingParams.LogoFileName);
			if(logo.GetLastStatus()==Ok)
			{
				int x,y;
				int logowidth,logoheight;
				logowidth=logo.GetWidth();
				logoheight=logo.GetHeight();
				if(m_imageConvertingParams.LogoPosition<3) y=0;
				else y=newheight-logoheight;
				if(m_imageConvertingParams.LogoPosition==0||m_imageConvertingParams.LogoPosition==3)
					x=0;
				if(m_imageConvertingParams.LogoPosition==2||m_imageConvertingParams.LogoPosition==5)
					x=newwidth-logowidth;
				if(m_imageConvertingParams.LogoPosition==1||m_imageConvertingParams.LogoPosition==4)
					x=(newwidth-logowidth)/2;

				gr.DrawImage(&logo, (int)x, (int)y,logowidth,logoheight);
			}
		}
		MySaveImage(&BackBuffer,GenerateFileName(L"img%md5.jpg",1,CPoint()),m_resultFileName,fileformat,m_imageConvertingParams.Quality);
	} 

	if(m_imageConvertingParams.KeepAsIs)

	{
		CString Ext = GetFileExt(sourceFile);
		if(Ext == _T("png"))
			fileformat = 1;
		else fileformat = 0;

	}
	if(/*thumbwidth && szBufferThumb*/m_generateThumb)
	{
		// Генерирование превьюшки с шаблоном в отдельной функции
		createThumb(&bm, fileformat);
		//GenThumb(szBufferImage,&bm, thumbwidth, newwidth, newheight, szBufferThumb, fileformat);
	}

	return 0;
	return true;
}

bool CImageConverter::createThumb(Gdiplus::Image *bm, int fileformat)
{
	bool result = false;
	HDC dc = ::GetDC(0);
	int newwidth = bm->GetWidth();
	int newheight = bm->GetHeight();
	int FileSize = MyGetFileSize(m_sourceFile);
	TCHAR SizeBuffer[100]=_T("\0");
	if(FileSize>0)
		NewBytesToString(FileSize,SizeBuffer,sizeof(SizeBuffer));

	CString ThumbnailText = m_thumbCreatingParams.Text; // Text that will be drawn on thumbnail

	ThumbnailText.Replace(_T("%width%"), IntToStr(newwidth)); //Replacing variables names with their values
	ThumbnailText.Replace(_T("%height%"), IntToStr(newheight));
	ThumbnailText.Replace(_T("%size%"), SizeBuffer);

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
		int thumbheight=((float)thumbwidth/(float)newwidth*newheight);


		int LabelHeight=TextRect.Height+1;
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
			RectF TextBounds(1,RealThumbHeight-LabelHeight,RealThumbWidth-1,LabelHeight+1);

			WCHAR Buffer[256];
			TCHAR SizeBuffer[100]=_T("\0");
		}

		if(fileformat == 2) 
			fileformat = 0;
		
		result = MySaveImage(&ThumbBuffer,_T("thumb"),m_thumbFileName,fileformat,93);
	}
	else
	{
		Graphics g1(dc);
		Font font(/*L"Tahoma", 10, FontStyleBold*/dc,&m_thumbCreatingParams.ThumbFont);

		RectF TextRect;

		FontFamily ff;
		font.GetFamily(&ff);
		g1.SetPageUnit(UnitPixel);
		g1.MeasureString(_T("test"),-1,&font,PointF(0,0),&TextRect);

		if(m_thumbCreatingParams.DrawFrame)
			thumbwidth-=2;
		int thumbheight=((float)thumbwidth/(float)newwidth*newheight);

		int LabelHeight=TextRect.Height+1;
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
			RectF TextBounds(1,RealThumbHeight-LabelHeight,RealThumbWidth-1,LabelHeight+1);

			thumbgr.SetPixelOffsetMode(PixelOffsetModeDefault );
			SolidBrush   br222(MYRGB(255,m_thumbCreatingParams.ThumbTextColor));
			DrawStrokedText(thumbgr, /*Buffer*/ThumbnailText,TextBounds,font,MYRGB(255,m_thumbCreatingParams.ThumbTextColor),MYRGB(90,0,0,0/*params.StrokeColor*/),1,1, 1);
		}

		thumbgr.SetPixelOffsetMode(   PixelOffsetModeHalf);
		thumbgr.SetSmoothingMode(SmoothingModeDefault);
		p.SetAlignment(PenAlignmentInset);

		if(m_thumbCreatingParams.DrawFrame)
			DrawRect(ThumbBuffer,MYRGB(255,m_thumbCreatingParams.FrameColor),Rect(0,0,RealThumbWidth,RealThumbHeight));

		if(fileformat == 2) 
			fileformat = 0;

		// Saving thumbnail (without template)
		result = MySaveImage(&ThumbBuffer,_T("thumb"),m_thumbFileName,fileformat,85);
	}
	ReleaseDC(0, dc);
	return result;
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
	TCHAR szImgTypes[3][4]={_T("jpg"),_T("png"),_T("gif")};
	TCHAR szMimeTypes[3][12]={_T("image/jpeg"),_T("image/png"),_T("image/gif")};
	CString szNameBuffer;
	TCHAR szBuffer2[MAX_PATH],TempPath[256];
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
	Bitmap temp(NewTextRect.Width,NewTextRect.Height,&gr);

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