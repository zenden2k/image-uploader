#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

#include "atlheaders.h"
#include <GdiPlus.h>
#include "Images/Thumbnail.h"
#include "Core/Utils/CoreTypes.h"



struct ImageConvertingParams
{
	enum ImageResizeMode { irmFit,  irmCrop, irmStretch };
	ImageConvertingParams();

	CString strNewWidth, strNewHeight;
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
	bool SmartConverting;
	CString LogoFileName;
	COLORREF TextColor, StrokeColor;
	EnumWrapper<ImageResizeMode> ResizeMode;
	bool PreserveExifInformation;
};

struct ThumbCreatingParams
{
	enum ThumbFormatEnum { tfSameAsImageFormat = 0, tfJPEG, tfPNG, tfGIF };
	enum ThumbResizeEnum { trByWidth = 0, trByHeight, trByBiggerSide };

	unsigned int Quality;
	CString Text;
	CString TemplateName;
	int Size;
	bool DrawFrame;
	bool AddImageSize;
	COLORREF BackgroundColor;
	EnumWrapper<ThumbFormatEnum> Format;
	ThumbResizeEnum ResizeMode;

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
		void setDestinationFolder(const CString& folder);
		void setGenerateThumb(bool generate);
		void setEnableProcessing(bool enable);
		void setImageConvertingParams(const ImageConvertingParams& params);
		void setThumbCreatingParams(const ThumbCreatingParams& params);
		void setThumbnail(Thumbnail* thumb);
		bool Convert(const CString& sourceFile);
		const CString getThumbFileName();
		const CString getImageFileName();
		bool createThumbnail(Gdiplus::Image* image, Gdiplus::Image** outResult, int64_t fileSize, int fileFormat = 1);
	protected:
		bool createThumb(Gdiplus::Image* bm, const CString& imageFile, int fileformat);
		Thumbnail* thumbnail_template_;
		std::map<std::string, std::string> m_Vars;
		bool EvaluateRect(const std::string& rectStr, RECT* out);
		int EvaluateExpression(const std::string& expr);
		unsigned int EvaluateColor(const std::string& expr);
		std::string ReplaceVars(const std::string& expr);
		int64_t EvaluateSimpleExpression(const std::string& expr) const;
		Gdiplus::Brush* CreateBrushFromString(const std::string& br,  RECT rect);
		bool processing_enabled;
};

bool MySaveImage(Gdiplus::Image* img, const CString& szFilename, CString& szBuffer, int Format, int Quality,
                 LPCTSTR Folder = 0);
void DrawGradient(Gdiplus::Graphics& gr, Gdiplus::Rect rect, Gdiplus::Color& Color1, Gdiplus::Color& Color2);
void DrawStrokedText(Gdiplus::Graphics& gr, LPCTSTR Text, Gdiplus::RectF Bounds, Gdiplus::Font& font,
                     Gdiplus::Color& ColorText, Gdiplus::Color& ColorStroke, int HorPos = 0, int VertPos = 0,
                     int width = 1);
#endif
