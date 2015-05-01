#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

#include "atlheaders.h"
#include "3rdpart/GdiplusH.h"
#include "Thumbnail.h"
#include "Core/Utils/CoreTypes.h"

class AbstractImage;

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
    uint32_t BackgroundColor;
    EnumWrapper<ThumbFormatEnum> Format;
    ThumbResizeEnum ResizeMode;
};

class ImageConverterPrivate;

class ImageConverter
{
    public:
        ImageConverter();
        ~ImageConverter();
        bool Convert(const std::string& sourceFile);
        const std::string getThumbFileName();
        const std::string getImageFileName();
        void setDestinationFolder(const std::string& folder);
        void setGenerateThumb(bool generate);
        void setEnableProcessing(bool enable);
        void setImageConvertingParams(const ImageConvertingParams& params);
        void setThumbCreatingParams(const ThumbCreatingParams& params);
        void setThumbnail(Thumbnail* thumb);
        std::shared_ptr<AbstractImage> createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat);
    protected:
        ImageConverterPrivate* d_ptr;
        Q_DECLARE_PRIVATE(ImageConverter);
private:
    DISALLOW_COPY_AND_ASSIGN(ImageConverter);

};
#endif
