#ifndef IU_IMAGECONVERTER_H
#define IU_IMAGECONVERTER_H

#include <string>

#include "Thumbnail.h"
#include "Core/Utils/CoreTypes.h"

class AbstractImage;

struct ImageConvertingParams
{
    enum ImageResizeMode { irmFit,  irmCrop, irmStretch };
    ImageConvertingParams();

    std::string strNewWidth, strNewHeight;
    bool AddText;
    std::string Text;
    int Format;
    int Quality;
    bool SaveProportions;
#ifdef _WIN32
    LOGFONT Font;
#endif
    bool AddLogo;
    int LogoPosition;
    int LogoBlend;
    int TextPosition;
    bool SmartConverting;
    std::string LogoFileName;
    uint32_t TextColor, StrokeColor;
    EnumWrapper<ImageResizeMode> ResizeMode;
    bool PreserveExifInformation;
    bool SkipAnimated;
};


struct ThumbCreatingParams
{
    enum ThumbFormatEnum { tfSameAsImageFormat = 0, tfJPEG, tfPNG, tfGIF, tfWebP, tfWebPLossless };
    enum ThumbResizeEnum { trByWidth = 0, trByHeight, trByBoth };

    unsigned int Quality;
    std::string Text;
    std::string TemplateName;
    int Size;
    int Width;
    int Height;
    bool DrawFrame;
    bool AddImageSize;
    uint32_t BackgroundColor;
    EnumWrapper<ThumbFormatEnum> Format;
    ThumbResizeEnum ResizeMode;

    static const int DEFAULT_THUMB_WIDTH = 180;

    ThumbCreatingParams() {
        Quality = 95;
        Size = 0;
        Width = 0;
        Height = 0;
        DrawFrame = true;
        AddImageSize = true;
        BackgroundColor = 0;
        Format = tfPNG;
        ResizeMode = trByWidth; 
    }
};

class ImageConverterPrivate;

class ImageConverter
{
    public:
        ImageConverter();
        ~ImageConverter();
        bool convert(const std::string& sourceFile);
        std::string getThumbFileName();
        std::string getImageFileName();
        void setDestinationFolder(const std::string& folder);
        void setGenerateThumb(bool generate);
        void setEnableProcessing(bool enable);
        void setImageConvertingParams(const ImageConvertingParams& params);
        void setThumbCreatingParams(const ThumbCreatingParams& params);
        void setThumbnail(Thumbnail* thumb);
        std::shared_ptr<AbstractImage> createThumbnail(AbstractImage* image, int64_t fileSize, int fileformat);
    protected:
        ImageConverterPrivate* d_ptr;
        MY_DECLARE_PRIVATE(ImageConverter);
private:
    DISALLOW_COPY_AND_ASSIGN(ImageConverter);
};
#endif
