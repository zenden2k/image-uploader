#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

#include <GdiPlus.h>
#include "Images/Thumbnail.h"
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

enum ThumbFormatEnum { tfSameAsImageFormat = 0, tfJPEG, tfPNG };

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
	ThumbFormatEnum ThumbFormat;
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
		void setThumbnail(Thumbnail * thumb);
		bool Convert(const CString& sourceFile);
		const CString getThumbFileName();
		const CString getImageFileName();
		bool createThumbnail(Gdiplus::Image *image, Gdiplus::Image ** outResult); 
	protected:
		bool createThumb(Gdiplus::Image *bm, int fileformat);
		Thumbnail* thumbnail_template_;
		std::map<std::string, std::string> m_Vars;
		bool EvaluateRect(const std::string& rectStr, RECT * out);
		int EvaluateExpression(const std::string& expr);
		unsigned int EvaluateColor(const std::string& expr);
		std::string ReplaceVars(const std::string& expr);
		zint64 EvaluateSimpleExpression(const std::string& expr) const;
		Gdiplus::Brush * CreateBrushFromString(const std::string& br,  RECT rect);
};

using namespace Gdiplus;
bool MySaveImage(Image *img, const CString& szFilename,CString& szBuffer,int Format,int Quality,LPCTSTR Folder=0);
void DrawGradient(Graphics &gr,Rect rect,Color &Color1,Color &Color2);
void DrawRect(Bitmap &gr,Color &color,Rect rect);
void DrawStrokedText(Graphics &gr, LPCTSTR Text,RectF Bounds,Gdiplus::Font &font,Color &ColorText,Color &ColorStroke,int HorPos=0,int VertPos=0, int width=1);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
#endif