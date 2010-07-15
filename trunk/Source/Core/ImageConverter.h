#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

struct ImageConvertingParams
{
	int NewWidth,NewHeight;
	BOOL KeepAsIs;
	BOOL AddText;
	CString Text;
	int Format;
	int Quality;
	BOOL SaveProportions;
	LOGFONT Font;
	BOOL AddLogo;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;
	CString LogoFileName;
	COLORREF TextColor,StrokeColor;
};

struct ThumbCreatingParams
{
	LOGFONT ThumbFont;
	int LogoPosition;
	int LogoBlend;
	int TextPosition;
	CString Text;
	CString FileName;
	//TCHAR FontName[256];
	COLORREF FrameColor,ThumbColor1,ThumbColor2/*TextBackground ,*/,ThumbTextColor;
	int ThumbAlpha;
	BOOL TextOverThumb;
	int ThumbWidth;
	BOOL UseThumbTemplate;
	BOOL DrawFrame;
	BOOL ThumbAddImageSize;
	BOOL ThumbAddBorder;	
};
class CImageConverter
{
	protected:
		CString m_destinationFolder;
		bool m_generateThumb;
		CString m_sourceFile;
		CString m_resultFileName;
		CString m_thumbFileName;
		ImageConvertingParams m_imageConvertingParams;
		ThumbCreatingParams m_thumbCreatingParams;
	public:
		CImageConverter();
		void setDestinationFolder(const CString &folder);
		void setGenerateThumb(bool generate);
		void setImageConvertingParams(const ImageConvertingParams &params);
		void setThumbCreatingParams(const ThumbCreatingParams &params);
		bool Convert(const CString& sourceFile);
		const CString getThumbFileName();
		const CString getImageFileName();
	protected:
		bool createThumb(Gdiplus::Image *bm, int fileformat);
};

bool MySaveImage(Image *img, const CString& szFilename,CString& szBuffer,int Format,int Quality,LPCTSTR Folder=0);
void DrawGradient(Graphics &gr,Rect rect,Color &Color1,Color &Color2);
void DrawRect(Bitmap &gr,Color &color,Rect rect);

#endif